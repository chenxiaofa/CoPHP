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

#ifndef PHP_COPHP_H
#define PHP_COPHP_H

extern zend_module_entry cophp_module_entry;
#define phpext_cophp_ptr &cophp_module_entry

#define PHP_COPHP_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_COPHP_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_COPHP_API __attribute__ ((visibility("default")))
#else
#	define PHP_COPHP_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif



struct _cothread_context{
	/* CoThread instance */
	zval* This;
	
	/* The suspended execution context. */
	zend_execute_data *execute_data;

	/* The separate stack used by cothread */
	zend_vm_stack stack;


	/* Return value */
	zval retval;
	/* Variable to put sent value into */
	//zval *send_target;


	/* Fake execute_data for stacktraces */
	/* zend_execute_data execute_fake; */

};

typedef struct _cothread_context cothread_context;


/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:

ZEND_BEGIN_MODULE_GLOBALS(cophp)
	zend_long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(cophp)
*/

/* Always refer to the globals in your function as COPHP_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
#define COPHP_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(cophp, v)

#if defined(ZTS) && defined(COMPILE_DL_COPHP)
ZEND_TSRMLS_CACHE_EXTERN()
#endif




#ifdef HAVE_GCC_GLOBAL_REGS
# if defined(__GNUC__) && ZEND_GCC_VERSION >= 4008 && defined(i386)
#  define ZEND_VM_FP_GLOBAL_REG "%esi"
#  define ZEND_VM_IP_GLOBAL_REG "%edi"
# elif defined(__GNUC__) && ZEND_GCC_VERSION >= 4008 && defined(__x86_64__)
#  define ZEND_VM_FP_GLOBAL_REG "%r14"
#  define ZEND_VM_IP_GLOBAL_REG "%r15"
# elif defined(__GNUC__) && ZEND_GCC_VERSION >= 4008 && defined(__powerpc64__)
#  define ZEND_VM_FP_GLOBAL_REG "r28"
#  define ZEND_VM_IP_GLOBAL_REG "r29"
# elif defined(__IBMC__) && ZEND_GCC_VERSION >= 4002 && defined(__powerpc64__)
#  define ZEND_VM_FP_GLOBAL_REG "r28"
#  define ZEND_VM_IP_GLOBAL_REG "r29"
# endif
#endif

#ifdef ZEND_VM_IP_GLOBAL_REG
#pragma GCC diagnostic ignored "-Wvolatile-register-var"
register const zend_op* volatile opline __asm__(ZEND_VM_IP_GLOBAL_REG);
#pragma GCC diagnostic warning "-Wvolatile-register-var"
#endif

#ifdef ZEND_VM_FP_GLOBAL_REG
#pragma GCC diagnostic ignored "-Wvolatile-register-var"
register zend_execute_data* volatile execute_data __asm__(ZEND_VM_FP_GLOBAL_REG);
#pragma GCC diagnostic warning "-Wvolatile-register-var"
#endif


#ifdef ZEND_VM_IP_GLOBAL_REG
# define OPLINE opline
# define USE_OPLINE
# define LOAD_OPLINE() opline = EX(opline)
# define LOAD_NEXT_OPLINE() opline = EX(opline) + 1
# define SAVE_OPLINE() EX(opline) = opline
#else
# define OPLINE EX(opline)
# define USE_OPLINE const zend_op *opline = EX(opline);
# define LOAD_OPLINE()
# define LOAD_NEXT_OPLINE() ZEND_VM_INC_OPCODE()
# define SAVE_OPLINE()
#endif




#endif	/* PHP_COPHP_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
