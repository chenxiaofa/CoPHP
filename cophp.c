/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "SAPI.h"

#include "php_ini.h"
#include "ext/standard/info.h"
#include "Zend/zend_extensions.h"
#include "Zend/zend_interfaces.h" /* for zend_call_method_with_* */

#include "php_cophp.h"

zend_execute_data *cothread_execute_data;
zend_execute_data *main_execute_data;
zend_string *run_function_name; 
cothread_context *current_cothread_context = NULL;
static void *cache_handler = NULL;
enum COTHREAD_STATUS{
	COTHREAD_STATUS_SUSPEND,
	COTHREAD_STATUS_RUNNING,
	COTHREAD_STATUS_DEAD,
	COTHREAD_IN_MAIN,
	COTHREAD_IN_COTHREAD
};

/* for Backup executor globals */
static zend_execute_data *original_execute_data;
static zend_class_entry *original_scope;
static zend_vm_stack original_stack;


void zend_always_inline cothread_backup_executor()
{
	/* Backup executor globals */
	original_execute_data = EG(current_execute_data);
	original_scope = EG(scope);
	original_stack = EG(vm_stack);
	original_stack->top = EG(vm_stack_top);

}

void zend_always_inline cothread_restore_executor()
{
	/* Restore executor globals */
	EG(current_execute_data) = original_execute_data;
	EG(scope) = original_scope;
	EG(vm_stack_top) = original_stack->top;
	EG(vm_stack_end) = original_stack->end;
	EG(vm_stack) = original_stack;
}


void ZEND_FASTCALL COTHREAD_SUSPEND_HANDLER(ZEND_OPCODE_HANDLER_ARGS)
{
	zend_op *next_op = (zend_op*)OPLINE;
	next_op->handler = cache_handler;
	SAVE_OPLINE();
	cache_handler = NULL;
	OPLINE = NULL;
	current_cothread_context->cothread_status = COTHREAD_STATUS_SUSPEND;
	current_cothread_context->execute_data = EG(current_execute_data);
	return;
}

//typedef void (ZEND_FASTCALL *opcode_handler_t) (ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);

void zend_always_inline cothread_free_vm_stack_call_frame(zend_execute_data *ex)
{
		
	while(ex)
	{ 
		/*
		if (ctx->cothread_status != COTHREAD_STATUS_DEAD)
		{//not finished cothread
			zend_execute_data *execute_data = ex;
			ex->opline = (zend_op *)(ex->func->op_array.opcodes + ex->func->op_array.last - 1);
			php_printf("ex last op:%ld handler=%ld\n",ex->opline,ex->opline->handler);


			EG(current_execute_data) = ex;
			EG(scope) = ex->func->common.scope;

			
			((opcode_handler_t)(ex->opline->handler))(ex);
		}
		*/
		//php_printf("free_vm_stack=%ld\n",ex);
		//free literals
		/*
		if (ex->func && ex->func->op_array.last_literal > 0)
		{
			int i = 0;
			for(i = 0 ; i < ex->func->op_array.last_literal; i++)
			{
				zval *z = (zval*)(ex->func->op_array.literals + i);
				php_printf("free_vm_stack=%ld - zval(%ld).gc =  \n",ex,z);
			}
		}
		*/
		zend_vm_stack_free_call_frame(ex);
		ex = ex->prev_execute_data;
	}
}


void zend_always_inline cothread_vm_stack_reset(zend_vm_stack stack)
{
	while (stack->prev != NULL) {
		zend_vm_stack p = stack->prev;
		efree(stack);
		stack = p;
	}
}





void zend_always_inline cothread_build_execute_data(cothread_context *ctx)
{
	cothread_backup_executor();
	

	ctx->cothread_status = COTHREAD_STATUS_SUSPEND;


	EG(vm_stack)	 = ctx->stack;
	EG(vm_stack_top) = ctx->stack->top;
	EG(vm_stack_end) = ctx->stack->end; 
 
	zend_execute_data *ex = ctx->execute_data?ctx->execute_data:ctx->top_execute_data;
	
	cothread_free_vm_stack_call_frame(ex);
	 
	cothread_vm_stack_reset(ctx->stack);

 	uint32_t call_info = ZEND_CALL_TOP_FUNCTION | ctx->fci_cache->function_handler->common.fn_flags ; 
	if (ctx->fci_cache->object) {
		call_info |= ZEND_CALL_RELEASE_THIS; 
	}

	 
	ctx->top_execute_data = ctx->execute_data = zend_vm_stack_push_call_frame(call_info, 
		(zend_function*)&ctx->fci_cache->function_handler->op_array, 
		0,
		ctx->fci_cache->called_scope,
		ctx->fci_cache->object
	);


	
	
	ctx->execute_data->symbol_table =  NULL;//zend_rebuild_symbol_table();

	/*
    HashTable *symbol_table =  emalloc(sizeof(zend_array));
	zend_hash_init(symbol_table, run_func->op_array.last_var, NULL, ZVAL_PTR_DTOR, 0);
	if (run_func->op_array.last_var) {
		
		zend_string **str = run_func->op_array.vars;
		zend_string **end = str + run_func->op_array.last_var;
		zval *var = ZEND_CALL_VAR_NUM(context->execute_data, 0);
		zend_hash_real_init(symbol_table, 0);

		do {
			_zend_hash_append_ind(symbol_table, *str, var);
			str++;
			var++;
		} while (str != end);
	}
	
	
	context->execute_data->symbol_table = symbol_table;

	*/
	i_init_execute_data(ctx->execute_data, &ctx->fci_cache->function_handler->op_array, NULL);




	ctx->stack      = EG(vm_stack); 
	ctx->stack->top = EG(vm_stack_top);
	ctx->stack->end = EG(vm_stack_end);
	

	cothread_restore_executor();
	
}




void zend_always_inline cothread_destory_context(cothread_context *ctx)
{
	//php_printf("cothread_destory_context(ctx=%X)",ctx);

	cothread_backup_executor();
	
	/* Set executor globals */

	EG(vm_stack_top) = ctx->stack->top;
	EG(vm_stack_end) = ctx->stack->end;
	EG(vm_stack) = ctx->stack;

	if (ZEND_CALL_INFO(ctx->top_execute_data) & ZEND_CALL_RELEASE_THIS)
	{
		zend_object *object = Z_OBJ(ctx->top_execute_data->This);
		OBJ_RELEASE(object);
	}

	zend_execute_data *ex = ctx->execute_data?ctx->execute_data:ctx->top_execute_data;
	cothread_free_vm_stack_call_frame(ex);

	//efree(ctx->top_execute_data->symbol_table);

	//php_printf("free_vm_stack %ld \n",EG(vm_stack));
	zend_vm_stack_destroy();
	efree(ctx->fci_cache);
	efree(ctx);
	
	cothread_restore_executor();
	
	
	
}



/* CoPHP class entry

*/
zend_class_entry *cothread_ce;


/**  CoPHP Method
*/

static zend_always_inline zend_vm_stack cothread_vm_stack_new_page() {
	zend_vm_stack page = (zend_vm_stack)emalloc(COTHREAD_VM_STACK_INIT_SIZE);
	page->top = ZEND_VM_STACK_ELEMETS(page);
	page->end = (zval*)((char*)page + COTHREAD_VM_STACK_INIT_SIZE);
	page->prev = NULL;
	return page;
}


static zend_always_inline void cothread_init_context(zend_object *object,cothread_context *ctx,zend_fcall_info_cache *fci_cache)
{

	ctx->stack = cothread_vm_stack_new_page();
	ctx->cothread_status = COTHREAD_STATUS_SUSPEND; 

	ctx->top_execute_data = ctx->execute_data = NULL;

	ctx->this_obj = object;
	
	ctx->fci_cache = fci_cache;

	cothread_build_execute_data(ctx);
}

void resume_cothread()
{

	/* Backup executor globals */
	cothread_backup_executor();
	
	/* Set executor globals */
	EG(current_execute_data) = current_cothread_context->execute_data;
	EG(scope) = current_cothread_context->execute_data->func->common.scope;
	EG(vm_stack_top) = current_cothread_context->stack->top;
	EG(vm_stack_end) = current_cothread_context->stack->end;
	EG(vm_stack) = current_cothread_context->stack;


	current_cothread_context->top_execute_data->prev_execute_data = original_execute_data;


	current_cothread_context->cothread_status = COTHREAD_STATUS_RUNNING;

	zend_execute_ex(current_cothread_context->execute_data);


	if (current_cothread_context->cothread_status == COTHREAD_STATUS_RUNNING)
	{		
		current_cothread_context->execute_data = NULL;
		current_cothread_context->cothread_status = COTHREAD_STATUS_DEAD;
	}
	current_cothread_context->top_execute_data->prev_execute_data = NULL;

	
	current_cothread_context->stack = EG(vm_stack);
	current_cothread_context->stack->top = EG(vm_stack_top);


	cothread_restore_executor();
	
	current_cothread_context = NULL;



}

ZEND_METHOD(cothread,__construct)
{

	zval *callback = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &callback) == FAILURE) { 
		return;
	}
	
	zend_fcall_info_cache *fci_cache;
	fci_cache = (zend_fcall_info_cache *)emalloc(sizeof(zend_fcall_info_cache));
	if (!fci_cache)
	{
		zend_error(E_ERROR,"alloc memory failed.");
		RETURN_FALSE;
	}
	zend_string *callable_name;
	char *error = NULL;
		
	if (!zend_is_callable_ex(callback,NULL,IS_CALLABLE_CHECK_SILENT,&callable_name,fci_cache,&error)){
		if (error) {
			zend_error(E_ERROR, "Invalid callback %s, %s", ZSTR_VAL(callable_name), error);
			efree(error);
		}
		if (callable_name) {
			zend_string_release(callable_name);
		}
		efree(fci_cache);
		RETURN_FALSE;
	} else if (error) {
		/* Capitalize the first latter of the error message */
		if (error[0] >= 'a' && error[0] <= 'z') {
			error[0] += ('A' - 'a');
		}
		zend_error(E_DEPRECATED, "%s", error);
		efree(error);
	}
	
	zend_string_release(callable_name);

	//创建cothread执行上下文
	cothread_context *context = (cothread_context *)emalloc(sizeof(cothread_context));
	zend_update_property_long(cothread_ce ,getThis(),"context",7,(zend_long)context); 
	zend_update_property(cothread_ce ,getThis(),"callable",8,callback); 
	cothread_init_context(Z_OBJ_P(getThis()),context,fci_cache);
	

//	php_printf("created cothread context:%ld\n",context);
//	php_printf("OBJ=%ld\n",Z_OBJ_P(getThis()));


}

ZEND_METHOD(cothread,__destruct)
{
	//php_printf("__destruct\n");
	zval rv; 
	zval *res = zend_read_property(cothread_ce,getThis(),"context",7,1,&rv);
	cothread_destory_context((cothread_context *)res->value.lval);
}

zend_always_inline cothread_context* current_context(zval *this)
{
	zval rv; 
	zval *res = zend_read_property(cothread_ce,this,"context",7,1,&rv);
	cothread_context *context = (cothread_context *)res->value.lval;
	return context;
}




ZEND_METHOD(cothread,yield) 
{ 
	//php_printf("current_execute_data:%ld\n",EG(current_execute_data));
	//LONGJMP(*EG(bailout), SUCCESS);

	if (current_cothread_context==NULL || current_cothread_context->cothread_status == COTHREAD_STATUS_SUSPEND)
	{
		RETURN_FALSE;
	}
	
	zend_op *next_op = (zend_op*)(OPLINE+1);
	cache_handler = (void*)next_op->handler;
	next_op->handler = COTHREAD_SUSPEND_HANDLER;
	

}

/*
ZEND_METHOD(cothread,start)
{

	if (current_cothread_context != NULL)
	{
		RETURN_NULL();
	}

	cothread_context * ctx = current_cothread_context = current_context(getThis());

	resume_cothread();

	zend_update_property_long(cothread_ce,getThis(),"status",6,ctx->cothread_status);
	
	
	
}
*/

ZEND_METHOD(cothread,reset)
{
	if (current_cothread_context != NULL )
	{
		RETURN_NULL();
	}
	
	cothread_context * ctx = current_context(getThis());
	
	if (ctx->cothread_status != COTHREAD_STATUS_DEAD)
	{
		RETURN_NULL();
	}

	cothread_build_execute_data(ctx);
	
}



ZEND_METHOD(cothread,running)
{
	if (current_cothread_context == NULL)
	{
		RETURN_NULL();
	}
	ZVAL_OBJ(return_value, current_cothread_context->this_obj);
	Z_TRY_ADDREF_P(return_value);
	return;
}


ZEND_METHOD(cothread,resume)
{

	
	if (current_cothread_context != NULL)
	{
		RETURN_NULL();
	}

	
	
	cothread_context * ctx = current_context(getThis());

	
	if (ctx->cothread_status == COTHREAD_STATUS_DEAD)
	{
		RETURN_NULL();
	}	
	current_cothread_context = ctx;
	resume_cothread();
	zend_update_property_long(cothread_ce,getThis(),"status",6,ctx->cothread_status);
}



/* CoPHP class method entry
*/
static zend_function_entry cothread_method[] = {

	ZEND_ME(cothread,  	yield						,  NULL,   ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(cothread,  	running		,  NULL,   ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(cothread,	__construct					,  NULL,   ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	ZEND_ME(cothread,	__destruct					,  NULL,   ZEND_ACC_PUBLIC|ZEND_ACC_DTOR|ZEND_ACC_FINAL)
	ZEND_ME(cothread,	resume						,  NULL,   ZEND_ACC_PUBLIC)
	ZEND_ME(cothread,	reset						,  NULL,   ZEND_ACC_PUBLIC)

    { NULL, NULL, NULL }
};


/* If you declare any globals in php_cophp.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(cophp)
*/

/* True global resources - no need for thread safety here */
static int le_cophp;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("cophp.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_cophp_globals, cophp_globals)
    STD_PHP_INI_ENTRY("cophp.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_cophp_globals, cophp_globals)
PHP_INI_END()
*/
/* }}} */



/* {{{ php_cophp_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_cophp_init_globals(zend_cophp_globals *cophp_globals)
{
	cophp_globals->global_value = 0;
	cophp_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(cophp)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/

	run_function_name = zend_string_init("run",3,1);

	
	zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "CoThread",cothread_method);
    cothread_ce = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_class_constant_long(cothread_ce, "STATUS_RUNNING", 	strlen("STATUS_RUNNING"),	COTHREAD_STATUS_RUNNING TSRMLS_CC);
	zend_declare_class_constant_long(cothread_ce, "STATUS_SUSPEND", 	strlen("STATUS_SUSPEND"),	COTHREAD_STATUS_SUSPEND TSRMLS_CC);
	zend_declare_class_constant_long(cothread_ce, "STATUS_DEAD", 		strlen("STATUS_DEAD"),		COTHREAD_STATUS_DEAD TSRMLS_CC);
	
    zend_declare_property_long(cothread_ce, "status", strlen("status"),COTHREAD_STATUS_SUSPEND, ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_long(cothread_ce, "context", strlen("context"),0, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_long(cothread_ce, "callable", strlen("callable"),0, ZEND_ACC_PRIVATE TSRMLS_CC);

	
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(cophp)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	zend_string_free(run_function_name);
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(cophp)
{
#if defined(COMPILE_DL_COPHP) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(cophp)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(cophp)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "cophp support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */



/* {{{ cophp_functions[]
 *
 * Every user visible function must have an entry in cophp_functions[].
 */
const zend_function_entry cophp_functions[] = {
	PHP_FE(cothread_test,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in cophp_functions[] */
};
/* }}} */

/* {{{ cophp_module_entry
 */
zend_module_entry cophp_module_entry = {
	STANDARD_MODULE_HEADER,
	"cophp",
	cophp_functions,
	PHP_MINIT(cophp),
	PHP_MSHUTDOWN(cophp),
	PHP_RINIT(cophp),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(cophp),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(cophp),
	PHP_COPHP_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_COPHP
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif



ZEND_DLEXPORT int cophp_zend_startup(zend_extension *extension)
{
	return zend_startup_module(&cophp_module_entry);
}



#ifndef ZEND_EXT_API
#define ZEND_EXT_API    ZEND_DLEXPORT
#endif

ZEND_EXTENSION();

ZEND_DLEXPORT zend_extension zend_extension_entry = {
	"CoPHP",
	"0.0.1",
	"Blod Chen",
	"",
	"",
	cophp_zend_startup,
	NULL,
	NULL,           /* activate_func_t */
	NULL,           /* deactivate_func_t */
	NULL,           /* message_handler_func_t */
	NULL,           /* op_array_handler_func_t */
	NULL, /* statement_handler_func_t */
	NULL,           /* fcall_begin_handler_func_t */
	NULL,           /* fcall_end_handler_func_t */
	NULL,   /* op_array_ctor_func_t */
	NULL,           /* op_array_dtor_func_t */
	STANDARD_ZEND_EXTENSION_PROPERTIES
};


ZEND_GET_MODULE(cophp)
#endif




/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
