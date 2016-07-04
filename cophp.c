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


void zend_always_inline cothread_destory_context(cothread_context *ctx)
{
	ctx->execute_data = NULL;

	//store vm stack 
	zend_vm_stack orig_vm_stack = EG(vm_stack);
	zval *orig_vm_top = EG(vm_stack_top);
	zval *orig_vm_end =	EG(vm_stack_end);

	EG(vm_stack)	 = ctx->stack;
	EG(vm_stack_top) = ctx->stack->top;
	EG(vm_stack_end) = ctx->stack->end; 

	
	zend_vm_stack_destroy();
	
	//restore vm stack
	EG(vm_stack)     = orig_vm_stack;
	EG(vm_stack_top) = orig_vm_top;
	EG(vm_stack_end) = orig_vm_end;
	
	efree(ctx);
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


static zend_always_inline void cothread_init_context(zend_object *object,cothread_context *context)
{

	context->stack = cothread_vm_stack_new_page();
 	ZVAL_NULL(&context->retval);

	zval *fz = zend_hash_find(EG(function_table),run_function_name);

	zend_function *run_func = (zend_function*)fz->value.ptr;
	
	zend_execute_data *cothread_execute_data;


	zend_vm_stack orig_vm_stack = EG(vm_stack);
	zval *orig_vm_top = EG(vm_stack_top);
	zval *orig_vm_end =	EG(vm_stack_end);

	EG(vm_stack)	 = context->stack;
	EG(vm_stack_top) = context->stack->top;
	EG(vm_stack_end) = context->stack->end; 
 

	
	context->execute_data = zend_vm_stack_push_call_frame(ZEND_CALL_TOP_CODE,
		(zend_function*)&run_func->op_array, 
		0,
		object->ce,
		object
	);
	
	context->execute_data->symbol_table = zend_rebuild_symbol_table();
	i_init_execute_data(context->execute_data, &run_func->op_array, &context->retval);



	context->stack      = EG(vm_stack);
	context->stack->top = EG(vm_stack_top);
	context->stack->end = EG(vm_stack_end);
	
	EG(vm_stack)	 = orig_vm_stack;
	EG(vm_stack_top) = orig_vm_top;
	EG(vm_stack_end) = orig_vm_end;

	

}

ZEND_METHOD(cothread,__construct)
{

	
	//创建cothread执行上下文
	cothread_context *context = (cothread_context *)emalloc(sizeof(cothread_context));
	zend_update_property_long(cothread_ce ,getThis(),"context",7,(zend_long)context); 
	cothread_init_context(Z_OBJ_P(getThis()),context);

	
	php_printf("created cothread context:%ld\n",context);
	php_printf("OBJ=%ld\n",Z_OBJ_P(getThis()));

/*
	zval *callback = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &callback) == FAILURE) {
		return;
	}

	php_printf("callback=%ld\n",callback);
*/

}

ZEND_METHOD(cothread,__destruct)
{
	php_printf("__destruct\n");
	zval rv; 
	zval *res = zend_read_property(cothread_ce,getThis(),"context",7,1,&rv);
	//cothread_destory_context((cothread_context *)res->value.lval);
}

zend_always_inline cothread_context* current_context(zval *this)
{
	zval rv; 
	zval *res = zend_read_property(cothread_ce,this,"context",7,1,&rv);
	cothread_context *context = (cothread_context *)res->value.lval;
	return context;
}




ZEND_METHOD(cothread,suspend) 
{ 
	//php_printf("current_execute_data:%ld\n",EG(current_execute_data));
	//LONGJMP(*EG(bailout), SUCCESS);
}

ZEND_METHOD(cothread,start)
{

	zend_object *this_obj = Z_OBJ_P(getThis());
	zend_class_entry *called_scope = this_obj->ce;
	
	zval ret;

	cothread_context *context = current_context(getThis());

	context->execute_data->prev_execute_data = EG(current_execute_data);
	
	
	
	
	zend_vm_stack orig_vm_stack = EG(vm_stack);
	zval *orig_vm_top = EG(vm_stack_top);
	zval *orig_vm_end =	EG(vm_stack_end);

	EG(vm_stack)	 = context->stack;
	EG(vm_stack_top) = context->stack->top;
	EG(vm_stack_end) = context->stack->end; 
 
 php_printf("vm_stack:%ld\n",context->stack); 
 php_printf("context->execute_data:%ld\n",context->execute_data); 
 php_printf("context->execute_data->This:%ld\n",&(context->execute_data->This)); 
 php_printf("Z_OBJ_P(context->execute_data->This):%ld\n",Z_OBJ_P(&(context->execute_data->This))); 
 php_printf("context->execute_data->called_scope:%ld\n",context->execute_data->called_scope); 

	ZVAL_NULL(&ret);
	zend_try {
		zend_execute_ex(context->execute_data);
		zend_vm_stack_free_call_frame(execute_data);
	} zend_catch {
		php_printf("zend_catch\n");
	} zend_end_try();

	//restore vm stack
	EG(vm_stack)	 = orig_vm_stack;
	EG(vm_stack_top) = orig_vm_top;
	EG(vm_stack_end) = orig_vm_end;
	
	
	
	
}


ZEND_METHOD(cothread,resume)
{

}


ZEND_METHOD(cothread,run){}

/* CoPHP class method entry
*/
static zend_function_entry cothread_method[] = {

	ZEND_ME(cothread,  	suspend		,  NULL,   ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(cothread,	__construct	,  NULL,   ZEND_ACC_PUBLIC|ZEND_ACC_CTOR|ZEND_ACC_FINAL)
	ZEND_ME(cothread,	__destruct	,  NULL,   ZEND_ACC_PUBLIC|ZEND_ACC_DTOR|ZEND_ACC_FINAL)
	//ZEND_ME(cothread,   run			,  NULL,   ZEND_ACC_PROTECTED|ZEND_ACC_ABSTRACT)
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
