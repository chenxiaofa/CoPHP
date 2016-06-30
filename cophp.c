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

#define PHP_COTHREAD_CONTEXT_RESOURCE_NAME "Cothread Context"
static int le_cothread_context;

static void php_cothread_context_dtor(zend_resource *res)
{

	//php_printf("free cothread context:%ld\n",res->ptr);
	cothread_context *context = (cothread_context *)res->ptr;

	context->execute_data = NULL;
	efree(context->stack);
	efree(res->ptr);

}




/*
zend_execute_data *cothread_create_execute_data(zend_execute_data *call, zend_op_array *op_array, zval *return_value) 
{
	 *
	 * Normally the execute_data is allocated on the VM stack (because it does
	 * not actually do any allocation and thus is faster). For generators
	 * though this behavior would be suboptimal, because the (rather large)
	 * structure would have to be copied back and forth every time execution is
	 * suspended or resumed. That's why for generators the execution context
	 * is allocated using a separate VM stack, thus allowing to save and
	 * restore it simply by replacing a pointer.
	 *
	zend_execute_data *execute_data;
	uint32_t num_args = 0
	size_t stack_size = (ZEND_CALL_FRAME_SLOT + MAX(op_array->last_var + op_array->T, num_args)) * sizeof(zval);
	uint32_t call_info;

	EG(vm_stack) = zend_vm_stack_new_page(
		EXPECTED(stack_size < ZEND_VM_STACK_FREE_PAGE_SIZE(1)) ?
			ZEND_VM_STACK_PAGE_SIZE(1) :
			ZEND_VM_STACK_PAGE_ALIGNED_SIZE(1, stack_size),
		NULL);
	EG(vm_stack_top) = EG(vm_stack)->top;
	EG(vm_stack_end) = EG(vm_stack)->end;

	call_info = ZEND_CALL_TOP_FUNCTION | ZEND_CALL_ALLOCATED | ZEND_CALL_RELEASE_THIS;
	//if (Z_OBJ(call->This)) {
	//	call_info |= ZEND_CALL_RELEASE_THIS;
	//}
	execute_data = zend_vm_stack_push_call_frame(
		call_info,
		(zend_function*)op_array,
		num_args,
		call->called_scope,
		Z_OBJ(call->This));
	EX(prev_execute_data) = NULL;
	EX_NUM_ARGS() = num_args;

	* copy arguments *
	if (num_args > 0) {
		zval *arg_src = ZEND_CALL_ARG(call, 1);
		zval *arg_dst = ZEND_CALL_ARG(execute_data, 1);
		zval *end = arg_src + num_args;

		do {
			ZVAL_COPY_VALUE(arg_dst, arg_src);
			arg_src++;
			arg_dst++;
		} while (arg_src != end);
	}

	EX(symbol_table) = NULL;

	i_init_func_execute_data(execute_data, op_array, return_value, 1);

	return execute_data;

}
*/

void cophp_gdb_break()
{

}


inline void jump_to_cothread(zval *this)
{
	zend_object *this_obj = Z_OBJ_P(this);
	
	zend_class_entry *called_scope = this_obj->ce;

	zval ret;

	ZVAL_NULL(&ret);
	zend_try {
		zend_call_method(this,called_scope,NULL,"run",3,&ret,0,NULL,NULL);
	} zend_catch {
		//php_printf("zend_catch\n");
	} zend_end_try();
}





inline void jump_to_cothread_(zval *this)
{
	
	
	zend_object *this_obj = Z_OBJ_P(this);
	
	zend_class_entry *called_scope = this_obj->ce;

	HashTable functions = called_scope->function_table;
	
	zval *run_function = zend_hash_find(&functions,run_function_name);

	if (NULL == run_function)
	{
		//php_printf("run function not found\n");
		return;
	}

	
	zend_function *func = (zend_function*)Z_PTR_P(run_function);


	

	// 直接执行
	//zend_execute(&func->op_array,NULL);
	

	
	zend_op_array *op_array = &func->op_array;
	zend_execute_data *execute_data;

	/*
	
	int op_index = 0;
	zend_op *tmp_op = NULL;
	for(;op_index < op_array->last; op_index++)
	{
		tmp_op = (zend_op*)(op_array->opcodes+op_index);
		//php_printf("zend_op.type=%d\n",tmp_op->opcode);
		if ( ZEND_RETURN == tmp_op->opcode )
		{
			//php_printf("zend_op.handler=%ld\n",tmp_op->handler);
			//tmp_op->handler = COTHREAD_RETURN_VALUE_HANDLER;
		}
	}
	*/
	
	//((zend_op*)(op_array->opcodes+op_array->last-1))->handler = COTHREAD_RETURN_VALUE_HANDLER;

	

  	execute_data = zend_vm_stack_push_call_frame(ZEND_CALL_TOP_FUNCTION, (zend_function*)op_array, 0, zend_get_called_scope(EG(current_execute_data)), zend_get_this_object(EG(current_execute_data)));
	if (EG(current_execute_data)) {
		execute_data->symbol_table = zend_rebuild_symbol_table();
	} else {
		execute_data->symbol_table = &EG(symbol_table);
	}
	EX(prev_execute_data) = EG(current_execute_data);
	i_init_execute_data(execute_data, op_array, NULL);
	//php_printf("%ld\n",execute_data);
	zend_execute_ex(execute_data);
	cophp_gdb_break();
	//zend_vm_stack_free_call_frame(execute_data);
	
	/**/

	/*
	zend_function *orig_prototype = func->common.prototype;
	zend_execute_data *call;

	uint32_t call_info = ZEND_CALL_NESTED_FUNCTION | ZEND_CALL_RELEASE_THIS;
	GC_REFCOUNT(this_obj)++; 
	
	zval *ret;
	//create call frame
 	call = zend_vm_stack_push_call_frame(call_info,	func, OPLINE->extended_value, called_scope, this_obj);
	call->prev_execute_data = NULL;
	//empty php return value
	ret = NULL;
	//empty php symbol table
	call->symbol_table = NULL;

	//set if php return value exists
	
	if (!((opline)->result_type & EXT_TYPE_UNUSED)) {
		ret = EX_VAR(opline->result.var);
		ZVAL_NULL(ret);
		Z_VAR_FLAGS_P(ret) = 0;
	}
	
	
	//call->prev_execute_data = execute_data;
	i_init_func_execute_data(call, &(func->op_array), ret, 1);

	//ZEND_ADD_CALL_FLAG(call, ZEND_CALL_TOP);
	//zend_execute_ex(call);


	//缓存执行环境
	zend_execute_data *_execute_data = execute_data;
	zend_op *_opline = OPLINE;

	execute_data = EG(current_execute_data); 
	LOAD_OPLINE(); 
	//php_printf("zend_execute_ex start\n");
	zend_execute_ex(execute_data);
	//php_printf("zend_execute_ex end\n");

	//恢复执行环境
	execute_data = _execute_data;
	OPLINE = _opline;

	
	/* */
	
}

/* CoPHP class entry

*/
zend_class_entry *cothread_ce;


/**  CoPHP Method
*/

void zend_always_inline cothread_init_context(cothread_context *context)
{
	//store vm stack 
	zend_vm_stack orig_vm_stack = EG(vm_stack);
	zval *orig_vm_top = EG(vm_stack_top);
	zval *orig_vm_end =	EG(vm_stack_end);

	zend_vm_stack_init();
	context->This = getThis();
	context->execute_data = NULL;
	context->stack = EG(vm_stack);
 	ZVAL_NULL(&context->retval);

	//restore vm stack
	EG(vm_stack)     = orig_vm_stack;
	EG(vm_stack_top) = orig_vm_top;
	EG(vm_stack_end) = orig_vm_end;
}

ZEND_METHOD(cothread,__construct)
{

	//创建cothread执行上下文
	cothread_context *context = (cothread_context *)emalloc(sizeof(cothread_context));
	//php_printf("created cothread context:%ld\n",context);
	zend_update_property(cothread_ce ,getThis(),"context",7,zend_list_insert((void*)context,le_cothread_context)); 
	cothread_init_context(context);


}

ZEND_METHOD(cothread,__destruct)
{

}

zend_always_inline cothread_context* current_context(zval *this)
{
	zval rv; 
	zval *res = zend_read_property(cothread_ce,this,"context",7,1,&rv);
	cothread_context *context = zend_fetch_resource(Z_RES_P(res),PHP_COTHREAD_CONTEXT_RESOURCE_NAME,le_cothread_context);
	return context;
}

ZEND_METHOD(cothread,run)
{

}


ZEND_METHOD(cothread,suspend) 
{ 
	php_printf("current_execute_data:%ld\n",EG(current_execute_data));
	LONGJMP(*EG(bailout), SUCCESS);
}


ZEND_METHOD(cothread,start)
{


	zval *this = getThis();
	zend_object *this_obj = Z_OBJ_P(this);
	
	zend_class_entry *called_scope = this_obj->ce;

	zval ret;

	cothread_context *context = current_context(getThis());

	zend_vm_stack orig_vm_stack = EG(vm_stack);
	zval *orig_vm_top = EG(vm_stack_top);
	zval *orig_vm_end =	EG(vm_stack_end);

	EG(vm_stack)	 = context->stack;
	EG(vm_stack_top) = context->stack->top;
	EG(vm_stack_end) = context->stack->end; 
 
	php_printf("vm_stack:%ld\n",context->stack); 
	
	ZVAL_NULL(&ret);
	zend_try {
		zend_call_method(this,called_scope,NULL,"run",3,&ret,0,NULL,NULL);
	} zend_catch {
		//php_printf("zend_catch\n");
	} zend_end_try();

	//restore vm stack
	EG(vm_stack)	 = orig_vm_stack;
	EG(vm_stack_top) = orig_vm_top;
	EG(vm_stack_end) = orig_vm_end;

	
	
	
}


ZEND_METHOD(cothread,resume)
{

}


/*
void zim_cothread_suspend(zend_execute_data *ex, zval *return_value)
{
	
	USE_OPLINE
		execute_data = cache_execute_data;
		OPLINE = cache_opline;
		ex->prev_execute_data = cache_execute_data;
	
	zend_execute_data *e = ex;
	while( e = e->prev_execute_data )
	{
		execute_data = e;
		OPLINE = execute_data->opline;
		ex->prev_execute_data = e;
	}
	
}
*/

/* CoPHP class method entry
*/
static zend_function_entry cothread_method[] = {

	ZEND_ME(cothread,  	suspend		,  NULL,   ZEND_ACC_PROTECTED)
	ZEND_ME(cothread,	__construct	,  NULL,   ZEND_ACC_PUBLIC|ZEND_ACC_CTOR|ZEND_ACC_FINAL)
	ZEND_ME(cothread,	__destruct	,  NULL,   ZEND_ACC_PUBLIC|ZEND_ACC_DTOR|ZEND_ACC_FINAL)
	ZEND_ME(cothread,   run			,  NULL,   ZEND_ACC_PROTECTED|ZEND_ACC_ABSTRACT)
	ZEND_ME(cothread,	start 		,  NULL,   ZEND_ACC_PUBLIC)
	ZEND_ME(cothread,	resume		,  NULL,   ZEND_ACC_PUBLIC)

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

	zend_declare_class_constant_long(cothread_ce, "STATUS_NEW", 		strlen("STATUS_NEW"),		0 TSRMLS_CC);
	zend_declare_class_constant_long(cothread_ce, "STATUS_RUNNABLE", 	strlen("STATUS_RUNNABLE"),	1 TSRMLS_CC);
	zend_declare_class_constant_long(cothread_ce, "STATUS_RUNNING", 	strlen("STATUS_RUNNING"),	2 TSRMLS_CC);
	zend_declare_class_constant_long(cothread_ce, "STATUS_SUSPEND", 	strlen("STATUS_SUSPEND"),	3 TSRMLS_CC);
	zend_declare_class_constant_long(cothread_ce, "STATUS_DEAD", 		strlen("STATUS_DEAD"),		4 TSRMLS_CC);
	
    zend_declare_property_long(cothread_ce, "status", strlen("status"),0, ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_long(cothread_ce, "context", strlen("context"),0, ZEND_ACC_PRIVATE TSRMLS_CC);

	
	le_cothread_context = zend_register_list_destructors_ex(php_cothread_context_dtor, NULL, PHP_COTHREAD_CONTEXT_RESOURCE_NAME,module_number);
	
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

PHP_FUNCTION(cothread_test)
{

}


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
