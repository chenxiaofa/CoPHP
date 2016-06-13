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
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_cophp.h"

static const zend_op *cache_opline;
static zend_execute_data *cache_execute_data;

/* CoPHP class entry

*/
zend_class_entry *cophp_ce;
void cophp_gdb_break()
{
}


ZEND_METHOD(cophp,record)
{
	cache_execute_data = execute_data->prev_execute_data;
	cache_opline = cache_execute_data->opline;
}


ZEND_METHOD(cophp,run)
{

}



void zim_cophp_suspend(zend_execute_data *ex, zval *return_value)
{
	USE_OPLINE
		execute_data = cache_execute_data;
		OPLINE = cache_opline;
		ex->prev_execute_data = cache_execute_data;
	/*
	zend_execute_data *e = ex;
	while( e = e->prev_execute_data )
	{
		execute_data = e;
		OPLINE = execute_data->opline;
		ex->prev_execute_data = e;
	}
	*/
}

/* CoPHP class method entry
*/
static zend_function_entry cophp_method[] = {
    ZEND_ME(cophp,    run	,  NULL,   ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(cophp,	  suspend		,  NULL,   ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(cophp,    record 		,  NULL,   ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)

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

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_cophp_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_cophp_compiled)
{
	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "cophp", arg);

	RETURN_STR(strg);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


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
	zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "CoPHP",cophp_method);
    cophp_ce = zend_register_internal_class(&ce TSRMLS_CC);
    zend_declare_property_null(cophp_ce, "public_var", strlen("public_var"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_stringl(cophp_ce, "private_var", strlen("private_var"),"string",strlen("string"), ZEND_ACC_PRIVATE TSRMLS_CC);
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
	//PHP_FE(confirm_cophp_compiled,	NULL)		/* For testing, remove later. */
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
