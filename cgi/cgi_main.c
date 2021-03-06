/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2008 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   |          Stig Bakken <ssb@php.net>                                   |
   |          Zeev Suraski <zeev@zend.com>                                |
   | FastCGI: Ben Mansell <php@slimyhorror.com>                           |
   |          Shane Caraveo <shane@caraveo.com>                           |
   |          Dmitry Stogov <dmitry@zend.com>                             |
   +----------------------------------------------------------------------+
*/

/* $Id: cgi_main.c,v 1.267.2.15.2.66 2008/11/28 11:56:50 dmitry Exp $ */

#include <php.h>
#include <php_globals.h>
#include <php_variables.h>
#include <zend_modules.h>

#include <SAPI.h>

#include <stdio.h>

#ifdef PHP_WIN32
#include "win32/time.h"
#include "win32/signal.h"
#include <process.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SIGNAL_H
#include <signal.h>
#endif
#if HAVE_SETLOCALE
#include <locale.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <zend.h>
#include <zend_extensions.h>
#include <php_ini.h>
#include <php_main.h>
#include <fopen_wrappers.h>
#include <ext/standard/php_standard.h>
#ifdef PHP_WIN32
#include <io.h>
#include <fcntl.h>
#include "win32/php_registry.h"
#endif

#ifdef __riscos__
#include <unixlib/local.h>
int __riscosify_control = __RISCOSIFY_STRICT_UNIX_SPECS;
#endif

#include "zend_compile.h"
#include "zend_execute.h"
#include "zend_highlight.h"
#include "zend_indent.h"

#include "php_getopt.h"

#include "fastcgi.h"

#ifdef FPM_AUTOCONFIG_H
#include <fpm_autoconfig.h>
#else
#include <php_config.h>
#endif
#include <fpm/fpm.h>
#include <fpm/fpm_request.h>


static void (*php_php_import_environment_variables)(zval *array_ptr TSRMLS_DC);

static int parent = 1;

static int request_body_fd;

static char *sapi_cgibin_getenv(char *name, size_t name_len TSRMLS_DC);

static char *php_optarg = NULL;
static int php_optind = 1;
static zend_module_entry cgi_module_entry;

static const opt_struct OPTIONS[] = {
	{'a', 0, "interactive"},
	{'b', 1, "bindpath"},
	{'C', 0, "no-chdir"},
	{'c', 1, "php-ini"},
	{'d', 1, "define"},
	{'e', 0, "profile-info"},
	{'f', 1, "file"},
	{'h', 0, "help"},
	{'i', 0, "info"},
	{'l', 0, "syntax-check"},
	{'m', 0, "modules"},
	{'n', 0, "no-php-ini"},
	{'q', 0, "no-header"},
	{'s', 0, "syntax-highlight"},
	{'s', 0, "syntax-highlighting"},
	{'w', 0, "strip"},
	{'?', 0, "usage"},/* help alias (both '?' and 'usage') */
	{'v', 0, "version"},
	{'x', 0, "fpm"},
	{'y', 1, "fpm-config"},
	{'z', 1, "zend-extension"},
	{'-', 0, NULL} /* end of args */
};

typedef struct _php_cgi_globals_struct {
	zend_bool rfc2616_headers;
	zend_bool nph;
	zend_bool check_shebang_line;
#if ENABLE_PATHINFO_CHECK
	zend_bool fix_pathinfo;
#endif
	zend_bool fcgi_logging;
# ifdef PHP_WIN32
	zend_bool impersonate;
# endif
	char *error_header;
} php_cgi_globals_struct;

#ifdef ZTS
static int php_cgi_globals_id;
#define CGIG(v) TSRMG(php_cgi_globals_id, php_cgi_globals_struct *, v)
#else
static php_cgi_globals_struct php_cgi_globals;
#define CGIG(v) (php_cgi_globals.v)
#endif

#ifdef PHP_WIN32
#define TRANSLATE_SLASHES(path) \
	{ \
		char *tmp = path; \
		while (*tmp) { \
			if (*tmp == '\\') *tmp = '/'; \
			tmp++; \
		} \
	}
#else
#define TRANSLATE_SLASHES(path)
#endif

static int print_module_info(zend_module_entry *module, void *arg TSRMLS_DC)
{
	php_printf("%s\n", module->name);
	return 0;
}

static int module_name_cmp(const void *a, const void *b TSRMLS_DC)
{
	Bucket *f = *((Bucket **) a);
	Bucket *s = *((Bucket **) b);

	return strcasecmp(((zend_module_entry *)f->pData)->name,
					  ((zend_module_entry *)s->pData)->name);
}

static void print_modules(TSRMLS_D)
{
	HashTable sorted_registry;
	zend_module_entry tmp;

	zend_hash_init(&sorted_registry, 50, NULL, NULL, 1);
	zend_hash_copy(&sorted_registry, &module_registry, NULL, &tmp, sizeof(zend_module_entry));
	zend_hash_sort(&sorted_registry, zend_qsort, module_name_cmp, 0 TSRMLS_CC);
	zend_hash_apply_with_argument(&sorted_registry, (apply_func_arg_t) print_module_info, NULL TSRMLS_CC);
	zend_hash_destroy(&sorted_registry);
}

static int print_extension_info(zend_extension *ext, void *arg TSRMLS_DC)
{
	php_printf("%s\n", ext->name);
	return 0;
}

static int extension_name_cmp(const zend_llist_element **f,
							  const zend_llist_element **s TSRMLS_DC)
{
	return strcmp(((zend_extension *)(*f)->data)->name,
				  ((zend_extension *)(*s)->data)->name);
}

static void print_extensions(TSRMLS_D)
{
	zend_llist sorted_exts;

	zend_llist_copy(&sorted_exts, &zend_extensions);
	sorted_exts.dtor = NULL;
	zend_llist_sort(&sorted_exts, extension_name_cmp TSRMLS_CC);
	zend_llist_apply_with_argument(&sorted_exts, (llist_apply_with_arg_func_t) print_extension_info, NULL TSRMLS_CC);
	zend_llist_destroy(&sorted_exts);
}

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

static inline size_t sapi_cgibin_single_write(const char *str, uint str_length TSRMLS_DC)
{
	long ret;

	if (fcgi_is_fastcgi()) {
		fcgi_request *request = (fcgi_request*) SG(server_context);
		long ret = fcgi_write(request, FCGI_STDOUT, str, str_length);
		if (ret <= 0) {
			return 0;
		}
		return ret;
	}
	ret = write(STDOUT_FILENO, str, str_length);
	if (ret <= 0) return 0;
	return ret;
}

static int sapi_cgibin_ub_write(const char *str, uint str_length TSRMLS_DC)
{
	const char *ptr = str;
	uint remaining = str_length;
	size_t ret;

	while (remaining > 0) {
		ret = sapi_cgibin_single_write(ptr, remaining TSRMLS_CC);
		if (!ret) {
			php_handle_aborted_connection();
			return str_length - remaining;
		}
		ptr += ret;
		remaining -= ret;
	}

	return str_length;
}


static void sapi_cgibin_flush(void *server_context)
{
	if (fcgi_is_fastcgi()) {
		fcgi_request *request = (fcgi_request*) server_context;
		if (
#ifndef PHP_WIN32
		!parent && 
#endif
		request && !fcgi_flush(request, 0)) {
			php_handle_aborted_connection();
		}
		return;
	}
	if (fflush(stdout) == EOF) {
		php_handle_aborted_connection();
	}
}

#define SAPI_CGI_MAX_HEADER_LENGTH 1024

typedef struct _http_error {
  int code;
  const char* msg;
} http_error;

static const http_error http_error_codes[] = {
	{100, "Continue"},
	{101, "Switching Protocols"},
	{200, "OK"},
	{201, "Created"},
	{202, "Accepted"},
	{203, "Non-Authoritative Information"},
	{204, "No Content"},
	{205, "Reset Content"},
	{206, "Partial Content"},
	{300, "Multiple Choices"},
	{301, "Moved Permanently"},
	{302, "Moved Temporarily"},
	{303, "See Other"},
	{304, "Not Modified"},
	{305, "Use Proxy"},
	{400, "Bad Request"},
	{401, "Unauthorized"},
	{402, "Payment Required"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{406, "Not Acceptable"},
	{407, "Proxy Authentication Required"},
	{408, "Request Time-out"},
	{409, "Conflict"},
	{410, "Gone"},
	{411, "Length Required"},
	{412, "Precondition Failed"},
	{413, "Request Entity Too Large"},
	{414, "Request-URI Too Large"},
	{415, "Unsupported Media Type"},
	{500, "Internal Server Error"},
	{501, "Not Implemented"},
	{502, "Bad Gateway"},
	{503, "Service Unavailable"},
	{504, "Gateway Time-out"},
	{505, "HTTP Version not supported"},
	{0,   NULL}
};

static int sapi_cgi_send_headers(sapi_headers_struct *sapi_headers TSRMLS_DC)
{
	char buf[SAPI_CGI_MAX_HEADER_LENGTH];
	sapi_header_struct *h;
	zend_llist_position pos;
	zend_bool ignore_status = 0;
	int response_status = SG(sapi_headers).http_response_code;

	if (SG(request_info).no_headers == 1) {
		return  SAPI_HEADER_SENT_SUCCESSFULLY;
	}

	if (CGIG(nph) || SG(sapi_headers).http_response_code != 200)
	{
		int len;
		zend_bool has_status = 0;

		if (CGIG(rfc2616_headers) && SG(sapi_headers).http_status_line) {
			char *s;
			len = slprintf(buf, SAPI_CGI_MAX_HEADER_LENGTH, "%s\r\n", SG(sapi_headers).http_status_line);
			if ((s = strchr(SG(sapi_headers).http_status_line, ' '))) {
				response_status = atoi((s + 1));
			}

			if (len > SAPI_CGI_MAX_HEADER_LENGTH) {
				len = SAPI_CGI_MAX_HEADER_LENGTH;
			}

		} else {
			char *s;

			if (SG(sapi_headers).http_status_line &&
			    (s = strchr(SG(sapi_headers).http_status_line, ' ')) != 0 &&
			    (s - SG(sapi_headers).http_status_line) >= 5 &&
			    strncasecmp(SG(sapi_headers).http_status_line, "HTTP/", 5) == 0) {
				len = slprintf(buf, sizeof(buf), "Status:%s\r\n", s);
				response_status = atoi((s + 1));
			} else {
				h = (sapi_header_struct*)zend_llist_get_first_ex(&sapi_headers->headers, &pos);
				while (h) {
					if (h->header_len > sizeof("Status:")-1 &&
					    strncasecmp(h->header, "Status:", sizeof("Status:")-1) == 0) {
						has_status = 1;
						break;
					}
					h = (sapi_header_struct*)zend_llist_get_next_ex(&sapi_headers->headers, &pos);
				}
				if (!has_status) {
					http_error *err = (http_error*)http_error_codes;

					while (err->code != 0) {
					    if (err->code == SG(sapi_headers).http_response_code) {
							break;
						}
						err++;
					}
					if (err->msg) {
						len = slprintf(buf, sizeof(buf), "Status: %d %s\r\n", SG(sapi_headers).http_response_code, err->msg);
					} else {
						len = slprintf(buf, sizeof(buf), "Status: %d\r\n", SG(sapi_headers).http_response_code);
					}
				}
			}
		}
		if (!has_status) {
			PHPWRITE_H(buf, len);
			ignore_status = 1;
		}
	}

	h = (sapi_header_struct*)zend_llist_get_first_ex(&sapi_headers->headers, &pos);
	while (h) {
		/* prevent CRLFCRLF */
		if (h->header_len) {
			if (h->header_len > sizeof("Status:")-1 &&
			    strncasecmp(h->header, "Status:", sizeof("Status:")-1) == 0) {
			    if (!ignore_status) {
				    ignore_status = 1;
					PHPWRITE_H(h->header, h->header_len);
					PHPWRITE_H("\r\n", 2);
				}
			} else if (response_status == 304 && h->header_len > sizeof("Content-Type:")-1 && 
					strncasecmp(h->header, "Content-Type:", sizeof("Content-Type:")-1) == 0) {
				h = (sapi_header_struct*)zend_llist_get_next_ex(&sapi_headers->headers, &pos);
				continue;
			} else {
				PHPWRITE_H(h->header, h->header_len);
				PHPWRITE_H("\r\n", 2);
			}
		}
		h = (sapi_header_struct*)zend_llist_get_next_ex(&sapi_headers->headers, &pos);
	}
	PHPWRITE_H("\r\n", 2);

	return SAPI_HEADER_SENT_SUCCESSFULLY;
}


static int sapi_cgi_read_post(char *buffer, uint count_bytes TSRMLS_DC)
{
	int read_bytes=0, tmp_read_bytes;

	count_bytes = MIN(count_bytes, (uint) SG(request_info).content_length - SG(read_post_bytes));
	while (read_bytes < count_bytes) {
		if (fcgi_is_fastcgi()) {
			fcgi_request *request = (fcgi_request*) SG(server_context);

			if (request_body_fd == -1) {
				char *request_body_filename = sapi_cgibin_getenv((char *) "REQUEST_BODY_FILE",
						sizeof("REQUEST_BODY_FILE")-1 TSRMLS_CC);

				if (request_body_filename && *request_body_filename) {
					request_body_fd = open(request_body_filename, O_RDONLY);

					if (0 > request_body_fd) {
						php_error(E_WARNING, "REQUEST_BODY_FILE: open('%s') failed: %s (%d)",
								request_body_filename, strerror(errno), errno);
						return 0;
					}
				}
			}

			/* If REQUEST_BODY_FILE variable not available - read post body from fastcgi stream */
			if (request_body_fd < 0) {
				tmp_read_bytes = fcgi_read(request, buffer + read_bytes, count_bytes - read_bytes);
			} else {
				tmp_read_bytes = read(request_body_fd, buffer + read_bytes, count_bytes - read_bytes);
			}
		} else {
			tmp_read_bytes = read(0, buffer + read_bytes, count_bytes - read_bytes);
		}

		if (tmp_read_bytes <= 0) {
			break;
		}
		read_bytes += tmp_read_bytes;
	}
	return read_bytes;
}

static char *sapi_cgibin_getenv(char *name, size_t name_len TSRMLS_DC)
{
	/* when php is started by mod_fastcgi, no regular environment
	   is provided to PHP.  It is always sent to PHP at the start
	   of a request.  So we have to do our own lookup to get env
	   vars.  This could probably be faster somehow.  */
	if (fcgi_is_fastcgi()) {
		fcgi_request *request = (fcgi_request*) SG(server_context);
		return fcgi_getenv(request, name, name_len);
	}
	/*  if cgi, or fastcgi and not found in fcgi env
		check the regular environment */
	return getenv(name);
}

static char *_sapi_cgibin_putenv(char *name, char *value TSRMLS_DC)
{
	int name_len;
#if !HAVE_SETENV || !HAVE_UNSETENV
	int len;
	char *buf;
#endif

	if (!name) {
		return NULL;
	}
	name_len = strlen(name);

	/* when php is started by mod_fastcgi, no regular environment
	   is provided to PHP.  It is always sent to PHP at the start
	   of a request.  So we have to do our own lookup to get env
	   vars.  This could probably be faster somehow.  */
	if (fcgi_is_fastcgi()) {
		fcgi_request *request = (fcgi_request*) SG(server_context);
		return fcgi_putenv(request, name, name_len, value);
	}
#if HAVE_SETENV
	if (value) {
		setenv(name, value, 1);
	}
#endif
#if HAVE_UNSETENV
	if (!value) {
		unsetenv(name);
	}
#endif

#if !HAVE_SETENV || !HAVE_UNSETENV
	/*  if cgi, or fastcgi and not found in fcgi env
		check the regular environment 
		this leaks, but it's only cgi anyway, we'll fix
		it for 5.0
	*/
	len = name_len + (value ? strlen(value) : 0) + sizeof("=") + 2;
	buf = (char *) malloc(len);
	if (buf == NULL) {
		return getenv(name);
	}
#endif
#if !HAVE_SETENV
	if (value) {
		len = slprintf(buf, len - 1, "%s=%s", name, value);
		putenv(buf);
	}
#endif
#if !HAVE_UNSETENV
	if (!value) {
		len = slprintf(buf, len - 1, "%s=", name);
		putenv(buf);
	}
#endif
	return getenv(name);
}

static char *sapi_cgi_read_cookies(TSRMLS_D)
{
	return sapi_cgibin_getenv((char *) "HTTP_COOKIE", sizeof("HTTP_COOKIE")-1 TSRMLS_CC);
}

void cgi_php_import_environment_variables(zval *array_ptr TSRMLS_DC)
{
	if (PG(http_globals)[TRACK_VARS_ENV] &&
	    array_ptr != PG(http_globals)[TRACK_VARS_ENV] &&
	    Z_TYPE_P(PG(http_globals)[TRACK_VARS_ENV]) == IS_ARRAY &&
	    zend_hash_num_elements(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_ENV])) > 0) {
	    zval_dtor(array_ptr);
	    *array_ptr = *PG(http_globals)[TRACK_VARS_ENV];
	    INIT_PZVAL(array_ptr);
	    zval_copy_ctor(array_ptr);
	    return;
	} else if (PG(http_globals)[TRACK_VARS_SERVER] &&
		array_ptr != PG(http_globals)[TRACK_VARS_SERVER] &&
	    Z_TYPE_P(PG(http_globals)[TRACK_VARS_SERVER]) == IS_ARRAY &&
	    zend_hash_num_elements(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER])) > 0) {
	    zval_dtor(array_ptr);
	    *array_ptr = *PG(http_globals)[TRACK_VARS_SERVER];
	    INIT_PZVAL(array_ptr);
	    zval_copy_ctor(array_ptr);
	    return;
	}
	
	/* call php's original import as a catch-all */
	php_php_import_environment_variables(array_ptr TSRMLS_CC);

	if (fcgi_is_fastcgi()) {
		fcgi_request *request = (fcgi_request*) SG(server_context);
		HashPosition pos;
		int magic_quotes_gpc = PG(magic_quotes_gpc);
		char *var, **val;
		uint var_len;
		ulong idx;
		int filter_arg = (array_ptr == PG(http_globals)[TRACK_VARS_ENV])?PARSE_ENV:PARSE_SERVER;

		/* turn off magic_quotes while importing environment variables */
		PG(magic_quotes_gpc) = 0;
		for (zend_hash_internal_pointer_reset_ex(&request->env, &pos);
		     zend_hash_get_current_key_ex(&request->env, &var, &var_len, &idx, 0, &pos) == HASH_KEY_IS_STRING &&
		     zend_hash_get_current_data_ex(&request->env, (void **) &val, &pos) == SUCCESS;
		     zend_hash_move_forward_ex(&request->env, &pos)) {
			unsigned int new_val_len;
			if (sapi_module.input_filter(filter_arg, var, val, strlen(*val), &new_val_len TSRMLS_CC)) {
				php_register_variable_safe(var, *val, new_val_len, array_ptr TSRMLS_CC);
			}
		}
		PG(magic_quotes_gpc) = magic_quotes_gpc;
	}
}

static void sapi_cgi_register_variables(zval *track_vars_array TSRMLS_DC)
{
	unsigned int php_self_len;
	char *php_self;

	/* In CGI mode, we consider the environment to be a part of the server
	 * variables
	 */
	php_import_environment_variables(track_vars_array TSRMLS_CC);

#if ENABLE_PATHINFO_CHECK
	if (CGIG(fix_pathinfo)) {
		char *script_name   = SG(request_info).request_uri;
		unsigned int script_name_len = script_name ? strlen(script_name) : 0;
		char *path_info     = sapi_cgibin_getenv("PATH_INFO", sizeof("PATH_INFO")-1 TSRMLS_CC);
		unsigned int path_info_len = path_info ? strlen(path_info) : 0;

		php_self_len = script_name_len + path_info_len;
		php_self = emalloc(php_self_len + 1);
		if (script_name) {
			memcpy(php_self, script_name, script_name_len + 1);
		}
		if (path_info) {
			memcpy(php_self + script_name_len, path_info, path_info_len + 1);
		}

		/* Build the special-case PHP_SELF variable for the CGI version */
		if (sapi_module.input_filter(PARSE_SERVER, "PHP_SELF", &php_self, php_self_len, &php_self_len TSRMLS_CC)) {
			php_register_variable_safe("PHP_SELF", php_self, php_self_len, track_vars_array TSRMLS_CC);
		}
		efree(php_self);
		return;
	}
#endif

	php_self = SG(request_info).request_uri ? SG(request_info).request_uri : "";
	php_self_len = strlen(php_self);
	if (sapi_module.input_filter(PARSE_SERVER, "PHP_SELF", &php_self, php_self_len, &php_self_len TSRMLS_CC)) {
		php_register_variable_safe("PHP_SELF", php_self, php_self_len, track_vars_array TSRMLS_CC);
	}
}

static void sapi_cgi_log_message(char *message)
{
	TSRMLS_FETCH();

	if (fcgi_is_fastcgi() && CGIG(fcgi_logging)) {
		fcgi_request *request;
		
		request = (fcgi_request*) SG(server_context);
		if (request) {			
			int len = strlen(message);
			char *buf = malloc(len+2);

			memcpy(buf, message, len);
			memcpy(buf + len, "\n", sizeof("\n"));
			fcgi_write(request, FCGI_STDERR, buf, len+1);
			free(buf);
		} else {
			fprintf(stderr, "%s\n", message);
		}
		/* ignore return code */
	} else
	fprintf(stderr, "%s\n", message);
}

static int sapi_cgi_deactivate(TSRMLS_D)
{
	/* flush only when SAPI was started. The reasons are:
		1. SAPI Deactivate is called from two places: module init and request shutdown
		2. When the first call occurs and the request is not set up, flush fails on 
			FastCGI.
	*/
	if (SG(sapi_started)) {
		sapi_cgibin_flush(SG(server_context));
	}
	return SUCCESS;
}

static int php_cgi_startup(sapi_module_struct *sapi_module)
{
	if (php_module_startup(sapi_module, &cgi_module_entry, 1) == FAILURE) {
		return FAILURE;
	}
	return SUCCESS;
}


/* {{{ sapi_module_struct cgi_sapi_module
 */
static sapi_module_struct cgi_sapi_module = {
	"cgi-fcgi",						/* name */
	"CGI/FastCGI",					/* pretty name */

	php_cgi_startup,				/* startup */
	php_module_shutdown_wrapper,	/* shutdown */

	NULL,							/* activate */
	sapi_cgi_deactivate,			/* deactivate */

	sapi_cgibin_ub_write,			/* unbuffered write */
	sapi_cgibin_flush,				/* flush */
	NULL,							/* get uid */
	sapi_cgibin_getenv,				/* getenv */

	php_error,						/* error handler */

	NULL,							/* header handler */
	sapi_cgi_send_headers,			/* send headers handler */
	NULL,							/* send header handler */

	sapi_cgi_read_post,				/* read POST data */
	sapi_cgi_read_cookies,			/* read Cookies */

	sapi_cgi_register_variables,	/* register server variables */
	sapi_cgi_log_message,			/* Log message */
	NULL,							/* Get request time */

	STANDARD_SAPI_MODULE_PROPERTIES
};
/* }}} */

/* {{{ php_cgi_usage
 */
static void php_cgi_usage(char *argv0)
{
	char *prog;

	prog = strrchr(argv0, '/');
	if (prog) {
		prog++;
	} else {
		prog = "php";
	}

	php_printf("Usage: %s [options]\n"
			   "\n"
			   "  -C               Do not chdir to the script's directory\n"
			   "  -c <path>|<file> Look for php.ini file in this directory\n"
			   "  -n               No php.ini file will be used\n"
			   "  -d foo[=bar]     Define INI entry foo with value 'bar'\n"
			   "  -e               Generate extended information for debugger/profiler\n"
			   "  -h               This help\n"
			   "  -i               PHP information\n"
			   "  -m               Show compiled in modules\n"
			   "  -v               Version number\n"
			   "  -y, --fpm-config <file>\n"
			   "                   Specify alternative path to FastCGI process manager config file.\n"
 			   "  -z <file>        Load Zend extension <file>.\n"
			   ,
			   prog);
}
/* }}} */

/* {{{ is_valid_path
 *
 * some server configurations allow '..' to slip through in the
 * translated path.   We'll just refuse to handle such a path.
 */
static int is_valid_path(const char *path)
{
	const char *p;

	if (!path) {
		return 0;
	}
	p = strstr(path, "..");
	if (p) {
		if ((p == path || IS_SLASH(*(p-1))) &&
		    (*(p+2) == 0 || IS_SLASH(*(p+2)))) {
			return 0;
		}
		while (1) {
			p = strstr(p+1, "..");
			if (!p) {
				break;
			}
			if (IS_SLASH(*(p-1)) &&
			    (*(p+2) == 0 || IS_SLASH(*(p+2)))) {
					return 0;
			}
		}
	}
	return 1;
}
/* }}} */

/* {{{ init_request_info

  initializes request_info structure

  specificly in this section we handle proper translations
  for:

  PATH_INFO
	derived from the portion of the URI path following 
	the script name but preceding any query data
	may be empty

  PATH_TRANSLATED
    derived by taking any path-info component of the 
	request URI and performing any virtual-to-physical 
	translation appropriate to map it onto the server's 
	document repository structure

	empty if PATH_INFO is empty

	The env var PATH_TRANSLATED **IS DIFFERENT** than the
	request_info.path_translated variable, the latter should
	match SCRIPT_FILENAME instead.

  SCRIPT_NAME
    set to a URL path that could identify the CGI script
	rather than the interpreter.  PHP_SELF is set to this.

  REQUEST_URI
    uri section following the domain:port part of a URI

  SCRIPT_FILENAME
    The virtual-to-physical translation of SCRIPT_NAME (as per 
	PATH_TRANSLATED)

  These settings are documented at
  http://cgi-spec.golux.com/


  Based on the following URL request:
  
  http://localhost/info.php/test?a=b 
 
  should produce, which btw is the same as if
  we were running under mod_cgi on apache (ie. not
  using ScriptAlias directives):
 
  PATH_INFO=/test
  PATH_TRANSLATED=/docroot/test
  SCRIPT_NAME=/info.php
  REQUEST_URI=/info.php/test?a=b
  SCRIPT_FILENAME=/docroot/info.php
  QUERY_STRING=a=b
 
  but what we get is (cgi/mod_fastcgi under apache):
  
  PATH_INFO=/info.php/test
  PATH_TRANSLATED=/docroot/info.php/test
  SCRIPT_NAME=/php/php-cgi  (from the Action setting I suppose)
  REQUEST_URI=/info.php/test?a=b
  SCRIPT_FILENAME=/path/to/php/bin/php-cgi  (Action setting translated)
  QUERY_STRING=a=b
 
  Comments in the code below refer to using the above URL in a request

 */
static void init_request_info(TSRMLS_D)
{
	char *env_script_filename = sapi_cgibin_getenv("SCRIPT_FILENAME", sizeof("SCRIPT_FILENAME")-1 TSRMLS_CC);
	char *env_path_translated = sapi_cgibin_getenv("PATH_TRANSLATED", sizeof("PATH_TRANSLATED")-1 TSRMLS_CC);
	char *script_path_translated = env_script_filename;

#if !DISCARD_PATH
	/* some broken servers do not have script_filename or argv0
	   an example, IIS configured in some ways.  then they do more
	   broken stuff and set path_translated to the cgi script location */
	if (!script_path_translated && env_path_translated) {
		script_path_translated = env_path_translated;
	}
#endif

	/* initialize the defaults */
	SG(request_info).path_translated = NULL;
	SG(request_info).request_method = NULL;
	SG(request_info).proto_num = 1000;
	SG(request_info).query_string = NULL;
	SG(request_info).request_uri = NULL;
	SG(request_info).content_type = NULL;
	SG(request_info).content_length = 0;
	SG(sapi_headers).http_response_code = 200;

	/* script_path_translated being set is a good indication that
	   we are running in a cgi environment, since it is always
	   null otherwise.  otherwise, the filename
	   of the script will be retreived later via argc/argv */
	if (script_path_translated) {
		const char *auth;
		char *content_length = sapi_cgibin_getenv("CONTENT_LENGTH", sizeof("CONTENT_LENGTH")-1 TSRMLS_CC);
		char *content_type = sapi_cgibin_getenv("CONTENT_TYPE", sizeof("CONTENT_TYPE")-1 TSRMLS_CC);
		char *env_path_info = sapi_cgibin_getenv("PATH_INFO", sizeof("PATH_INFO")-1 TSRMLS_CC);
		char *env_script_name = sapi_cgibin_getenv("SCRIPT_NAME", sizeof("SCRIPT_NAME")-1 TSRMLS_CC);
#if ENABLE_PATHINFO_CHECK
		struct stat st;
		char *env_redirect_url = sapi_cgibin_getenv("REDIRECT_URL", sizeof("REDIRECT_URL")-1 TSRMLS_CC);
		char *env_document_root = sapi_cgibin_getenv("DOCUMENT_ROOT", sizeof("DOCUMENT_ROOT")-1 TSRMLS_CC);
		int script_path_translated_len;

		/* Hack for buggy IIS that sets incorrect PATH_INFO */
		char *env_server_software = sapi_cgibin_getenv("SERVER_SOFTWARE", sizeof("SERVER_SOFTWARE")-1 TSRMLS_CC);
		if (env_server_software &&
		    env_script_name &&
		    env_path_info &&
		    strncmp(env_server_software, "Microsoft-IIS", sizeof("Microsoft-IIS")-1) == 0 &&
		    strncmp(env_path_info, env_script_name, strlen(env_script_name)) == 0) {
			env_path_info = _sapi_cgibin_putenv("ORIG_PATH_INFO", env_path_info TSRMLS_CC);
		    env_path_info += strlen(env_script_name);
		    if (*env_path_info == 0) {
		    	env_path_info = NULL;
		    }
			env_path_info = _sapi_cgibin_putenv("PATH_INFO", env_path_info TSRMLS_CC);
		}

		if (CGIG(fix_pathinfo)) {
			char *real_path = NULL;
			char *orig_path_translated = env_path_translated;
			char *orig_path_info = env_path_info;
			char *orig_script_name = env_script_name;
			char *orig_script_filename = env_script_filename;

			if (!env_document_root && PG(doc_root)) {
				env_document_root = _sapi_cgibin_putenv("DOCUMENT_ROOT", PG(doc_root) TSRMLS_CC);
				/* fix docroot */
				TRANSLATE_SLASHES(env_document_root);
 			}

			if (env_path_translated != NULL && env_redirect_url != NULL) {
				/* 
				   pretty much apache specific.  If we have a redirect_url
				   then our script_filename and script_name point to the
				   php executable
				*/
				script_path_translated = env_path_translated;
				/* we correct SCRIPT_NAME now in case we don't have PATH_INFO */
				env_script_name = env_redirect_url;
			}

#ifdef __riscos__
			/* Convert path to unix format*/
			__riscosify_control |= __RISCOSIFY_DONT_CHECK_DIR;
			script_path_translated = __unixify(script_path_translated, 0, NULL, 1, 0);
#endif
			
			/*
			 * if the file doesn't exist, try to extract PATH_INFO out
			 * of it by stat'ing back through the '/'
			 * this fixes url's like /info.php/test
			 */
			if (script_path_translated &&
				(script_path_translated_len = strlen(script_path_translated)) > 0 &&
				(script_path_translated[script_path_translated_len-1] == '/' ||
#ifdef PHP_WIN32
				 script_path_translated[script_path_translated_len-1] == '\\' ||
#endif
			     (real_path = tsrm_realpath(script_path_translated, NULL TSRMLS_CC)) == NULL)) {
				char *pt = estrndup(script_path_translated, script_path_translated_len);
				int len = script_path_translated_len;
				char *ptr;

				while ((ptr = strrchr(pt, '/')) || (ptr = strrchr(pt, '\\'))) {
					*ptr = 0;
					if (stat(pt, &st) == 0 && S_ISREG(st.st_mode)) {
						/*
						 * okay, we found the base script!
						 * work out how many chars we had to strip off;
						 * then we can modify PATH_INFO
						 * accordingly
						 *
						 * we now have the makings of
						 * PATH_INFO=/test
						 * SCRIPT_FILENAME=/docroot/info.php
						 *
						 * we now need to figure out what docroot is.
						 * if DOCUMENT_ROOT is set, this is easy, otherwise,
						 * we have to play the game of hide and seek to figure
						 * out what SCRIPT_NAME should be
						 */
						int slen = len - strlen(pt);
						int pilen = env_path_info ? strlen(env_path_info) : 0;
						char *path_info = env_path_info ? env_path_info + pilen - slen : NULL;

						if (orig_path_info != path_info) {
							if (orig_path_info) {
								char old;

								_sapi_cgibin_putenv("ORIG_PATH_INFO", orig_path_info TSRMLS_CC);
								old = path_info[0];
								path_info[0] = 0;
								if (!orig_script_name ||
									strcmp(orig_script_name, env_path_info) != 0) {
									if (orig_script_name) {
										_sapi_cgibin_putenv("ORIG_SCRIPT_NAME", orig_script_name TSRMLS_CC);
									}
									SG(request_info).request_uri = _sapi_cgibin_putenv("SCRIPT_NAME", env_path_info TSRMLS_CC);
								} else {
									SG(request_info).request_uri = orig_script_name;
								}
								path_info[0] = old;
							}
							env_path_info = _sapi_cgibin_putenv("PATH_INFO", path_info TSRMLS_CC);
						}
						if (!orig_script_filename ||
							strcmp(orig_script_filename, pt) != 0) {
							if (orig_script_filename) {
								_sapi_cgibin_putenv("ORIG_SCRIPT_FILENAME", orig_script_filename TSRMLS_CC);
							}
							script_path_translated = _sapi_cgibin_putenv("SCRIPT_FILENAME", pt TSRMLS_CC);
						}
						TRANSLATE_SLASHES(pt);

						/* figure out docroot
						   SCRIPT_FILENAME minus SCRIPT_NAME
						*/

						if (env_document_root) {
							int l = strlen(env_document_root);
							int path_translated_len = 0;
							char *path_translated = NULL;
							
							if (l && env_document_root[l - 1] == '/') {
								--l;
							}

							/* we have docroot, so we should have:
							 * DOCUMENT_ROOT=/docroot
							 * SCRIPT_FILENAME=/docroot/info.php
							 */

							/* PATH_TRANSLATED = DOCUMENT_ROOT + PATH_INFO */
							path_translated_len = l + (env_path_info ? strlen(env_path_info) : 0);
							path_translated = (char *) emalloc(path_translated_len + 1);
							memcpy(path_translated, env_document_root, l);
							if (env_path_info) {
								memcpy(path_translated + l, env_path_info, (path_translated_len - l));
							}
							path_translated[path_translated_len] = '\0';
							if (orig_path_translated) {
								_sapi_cgibin_putenv("ORIG_PATH_TRANSLATED", orig_path_translated TSRMLS_CC);
						   	}
							env_path_translated = _sapi_cgibin_putenv("PATH_TRANSLATED", path_translated TSRMLS_CC);
							efree(path_translated);
						} else if (env_script_name && 
								   strstr(pt, env_script_name)
						) {
							/* PATH_TRANSLATED = PATH_TRANSLATED - SCRIPT_NAME + PATH_INFO */
							int ptlen = strlen(pt) - strlen(env_script_name);
							int path_translated_len = ptlen + (env_path_info ? strlen(env_path_info) : 0);
							char *path_translated = NULL;

							path_translated = (char *) emalloc(path_translated_len + 1);
							memcpy(path_translated, pt, ptlen);
							if (env_path_info) {
								memcpy(path_translated + ptlen, env_path_info, path_translated_len - ptlen);
							}
							path_translated[path_translated_len] = '\0';
							if (orig_path_translated) {
								_sapi_cgibin_putenv("ORIG_PATH_TRANSLATED", orig_path_translated TSRMLS_CC);
						   	}
							env_path_translated = _sapi_cgibin_putenv("PATH_TRANSLATED", path_translated TSRMLS_CC);
							efree(path_translated);
						}
						break;
					}
				}
				if (!ptr) {
					/*
					 * if we stripped out all the '/' and still didn't find
					 * a valid path... we will fail, badly. of course we would
					 * have failed anyway... we output 'no input file' now.
					 */
					if (orig_script_filename) {
						_sapi_cgibin_putenv("ORIG_SCRIPT_FILENAME", orig_script_filename TSRMLS_CC);
					}
					script_path_translated = _sapi_cgibin_putenv("SCRIPT_FILENAME", NULL TSRMLS_CC);
					SG(sapi_headers).http_response_code = 404;
				}
				if (!SG(request_info).request_uri) {
					if (!orig_script_name ||
						strcmp(orig_script_name, env_script_name) != 0) {
						if (orig_script_name) {
							_sapi_cgibin_putenv("ORIG_SCRIPT_NAME", orig_script_name TSRMLS_CC);
						}
						SG(request_info).request_uri = _sapi_cgibin_putenv("SCRIPT_NAME", env_script_name TSRMLS_CC);
					} else {
						SG(request_info).request_uri = orig_script_name;
					}
				}	
				if (pt) {
					efree(pt);
				}
				if (is_valid_path(script_path_translated)) {
					SG(request_info).path_translated = estrdup(script_path_translated);
				}
			} else {
				/* make sure path_info/translated are empty */
				if (!orig_script_filename ||
					(script_path_translated != orig_script_filename &&
					strcmp(script_path_translated, orig_script_filename) != 0)) {
					if (orig_script_filename) {
						_sapi_cgibin_putenv("ORIG_SCRIPT_FILENAME", orig_script_filename TSRMLS_CC);
					}
					script_path_translated = _sapi_cgibin_putenv("SCRIPT_FILENAME", script_path_translated TSRMLS_CC);
				}
				if (env_redirect_url) {
	 				if (orig_path_info) {
						_sapi_cgibin_putenv("ORIG_PATH_INFO", orig_path_info TSRMLS_CC);
						_sapi_cgibin_putenv("PATH_INFO", NULL TSRMLS_CC);
					}
					if (orig_path_translated) {
	 					_sapi_cgibin_putenv("ORIG_PATH_TRANSLATED", orig_path_translated TSRMLS_CC);
						_sapi_cgibin_putenv("PATH_TRANSLATED", NULL TSRMLS_CC);
					}
				}
				if (env_script_name != orig_script_name) {
					if (orig_script_name) {
						_sapi_cgibin_putenv("ORIG_SCRIPT_NAME", orig_script_name TSRMLS_CC);
					}
					SG(request_info).request_uri = _sapi_cgibin_putenv("SCRIPT_NAME", env_script_name TSRMLS_CC);
				} else {
					SG(request_info).request_uri = env_script_name;
				}
				if (is_valid_path(script_path_translated)) {
					SG(request_info).path_translated = estrdup(script_path_translated);
				}
				free(real_path);
			}
		} else {
#endif
			/* pre 4.3 behaviour, shouldn't be used but provides BC */
			if (env_path_info) {
				SG(request_info).request_uri = env_path_info;
			} else {
				SG(request_info).request_uri = env_script_name;
			}
#if !DISCARD_PATH
			if (env_path_translated) {
				script_path_translated = env_path_translated;
			}
#endif
			if (is_valid_path(script_path_translated)) {
				SG(request_info).path_translated = estrdup(script_path_translated);
			}
#if ENABLE_PATHINFO_CHECK
		}
#endif
		SG(request_info).request_method = sapi_cgibin_getenv("REQUEST_METHOD", sizeof("REQUEST_METHOD")-1 TSRMLS_CC);
		/* FIXME - Work out proto_num here */
		SG(request_info).query_string = sapi_cgibin_getenv("QUERY_STRING", sizeof("QUERY_STRING")-1 TSRMLS_CC);
		SG(request_info).content_type = (content_type ? content_type : "" );
		SG(request_info).content_length = (content_length ? atoi(content_length) : 0);
		
		/* The CGI RFC allows servers to pass on unvalidated Authorization data */
		auth = sapi_cgibin_getenv("HTTP_AUTHORIZATION", sizeof("HTTP_AUTHORIZATION")-1 TSRMLS_CC);
		php_handle_auth_data(auth TSRMLS_CC);
	}
}
/* }}} */


PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("cgi.rfc2616_headers",     "0",  PHP_INI_ALL,    OnUpdateBool,   rfc2616_headers, php_cgi_globals_struct, php_cgi_globals)
	STD_PHP_INI_ENTRY("cgi.nph",                 "0",  PHP_INI_ALL,    OnUpdateBool,   nph, php_cgi_globals_struct, php_cgi_globals)
	STD_PHP_INI_ENTRY("cgi.check_shebang_line",  "1",  PHP_INI_SYSTEM, OnUpdateBool,   check_shebang_line, php_cgi_globals_struct, php_cgi_globals)
#if ENABLE_PATHINFO_CHECK
	STD_PHP_INI_ENTRY("cgi.fix_pathinfo",        "1",  PHP_INI_SYSTEM, OnUpdateBool,   fix_pathinfo, php_cgi_globals_struct, php_cgi_globals)
#endif
	STD_PHP_INI_ENTRY("fastcgi.logging",         "1",  PHP_INI_SYSTEM, OnUpdateBool,   fcgi_logging, php_cgi_globals_struct, php_cgi_globals)
# ifdef PHP_WIN32
	STD_PHP_INI_ENTRY("fastcgi.impersonate",     "0",  PHP_INI_SYSTEM, OnUpdateBool,   impersonate, php_cgi_globals_struct, php_cgi_globals)
# endif
	STD_PHP_INI_ENTRY("fastcgi.error_header",    NULL, PHP_INI_SYSTEM, OnUpdateString, error_header, php_cgi_globals_struct, php_cgi_globals)
PHP_INI_END()

/* {{{ php_cgi_globals_ctor
 */
static void php_cgi_globals_ctor(php_cgi_globals_struct *php_cgi_globals TSRMLS_DC)
{
	php_cgi_globals->rfc2616_headers = 0;
	php_cgi_globals->nph = 0;
	php_cgi_globals->check_shebang_line = 1;
#if ENABLE_PATHINFO_CHECK
	php_cgi_globals->fix_pathinfo = 1;
#endif
	php_cgi_globals->fcgi_logging = 1;
# ifdef PHP_WIN32
	php_cgi_globals->impersonate = 0;
# endif
	php_cgi_globals->error_header = NULL;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
static PHP_MINIT_FUNCTION(cgi)
{
#ifdef ZTS
	ts_allocate_id(&php_cgi_globals_id, sizeof(php_cgi_globals_struct), (ts_allocate_ctor) php_cgi_globals_ctor, NULL);
#else
	php_cgi_globals_ctor(&php_cgi_globals TSRMLS_CC);
#endif
	REGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
static PHP_MSHUTDOWN_FUNCTION(cgi)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
static PHP_MINFO_FUNCTION(cgi)
{
	DISPLAY_INI_ENTRIES();

	php_info_print_table_start();
	php_info_print_table_row(2, "php-fpm", "active");
	php_info_print_table_row(2, "php-fpm version", PHP_FPM_VERSION);
	php_info_print_table_end();

}
/* }}} */

PHP_FUNCTION(fastcgi_finish_request)
{
	fcgi_request *request = (fcgi_request*) SG(server_context);

	if (fcgi_is_fastcgi() && request->fd >= 0) {

		php_end_ob_buffers(1 TSRMLS_CC);
		php_header(TSRMLS_C);

		fcgi_flush(request, 1);
		fcgi_close(request, 0, 0);
		RETURN_TRUE;
	}

	RETURN_FALSE;

}

function_entry cgi_fcgi_sapi_functions[] = {
	PHP_FE(fastcgi_finish_request,				NULL)
	{NULL, NULL, NULL}
};

static zend_module_entry cgi_module_entry = {
	STANDARD_MODULE_HEADER,
	"cgi-fcgi",
	cgi_fcgi_sapi_functions, 
	PHP_MINIT(cgi), 
	PHP_MSHUTDOWN(cgi), 
	NULL, 
	NULL, 
	PHP_MINFO(cgi), 
	NO_VERSION_YET,
	STANDARD_MODULE_PROPERTIES
};

/* {{{ main
 */
int main(int argc, char *argv[])
{
	int free_query_string = 0;
	int exit_status = SUCCESS;
	int c;
	zend_file_handle file_handle = {};
	int retval;
/* temporary locals */
	int orig_optind = php_optind;
	char *orig_optarg = php_optarg;
	int ini_entries_len = 0;

/* end of temporary locals */
#ifdef ZTS
	void ***tsrm_ls;
#endif

	int max_requests = 500;
	int requests = 0;
	int fcgi_fd = 0;
	fcgi_request request;
	char *fpm_config = NULL;

#ifdef HAVE_SIGNAL_H
#if defined(SIGPIPE) && defined(SIG_IGN)
	signal(SIGPIPE, SIG_IGN); /* ignore SIGPIPE in standalone mode so
								that sockets created via fsockopen()
								don't kill PHP if the remote site
								closes it.  in apache|apxs mode apache
								does that for us!  thies@thieso.net
								20000419 */
#endif
#endif

#ifdef ZTS
	tsrm_startup(1, 1, 0, NULL);
	tsrm_ls = ts_resource(0);
#endif

	sapi_startup(&cgi_sapi_module);
	cgi_sapi_module.php_ini_path_override = NULL;

#ifdef PHP_WIN32
	_fmode = _O_BINARY; /* sets default for file streams to binary */
	setmode(_fileno(stdin),  O_BINARY);	/* make the stdio mode be binary */
	setmode(_fileno(stdout), O_BINARY);	/* make the stdio mode be binary */
	setmode(_fileno(stderr), O_BINARY);	/* make the stdio mode be binary */
#endif

	while ((c = php_getopt(argc, argv, OPTIONS, &php_optarg, &php_optind, 0)) != -1) {
		switch (c) {

			case 'c':
				if (cgi_sapi_module.php_ini_path_override) {
					free(cgi_sapi_module.php_ini_path_override);
				}
				cgi_sapi_module.php_ini_path_override = strdup(php_optarg);
				break;

			case 'n':
				cgi_sapi_module.php_ini_ignore = 1;
				break;

			case 'C': /* don't chdir to the script directory */
				SG(options) |= SAPI_OPTION_NO_CHDIR;
				break;

			case 'd': { 
				/* define ini entries on command line */
				int len = strlen(php_optarg);
				char *val;

				if ((val = strchr(php_optarg, '='))) {
					val++;
					if (!isalnum(*val) && *val != '"' && *val != '\'' && *val != '\0') {
						cgi_sapi_module.ini_entries = realloc(cgi_sapi_module.ini_entries, ini_entries_len + len + sizeof("\"\"\n\0"));
						memcpy(cgi_sapi_module.ini_entries + ini_entries_len, php_optarg, (val - php_optarg));
						ini_entries_len += (val - php_optarg);
						memcpy(cgi_sapi_module.ini_entries + ini_entries_len, "\"", 1);
						ini_entries_len++;
						memcpy(cgi_sapi_module.ini_entries + ini_entries_len, val, len - (val - php_optarg));
						ini_entries_len += len - (val - php_optarg);
						memcpy(cgi_sapi_module.ini_entries + ini_entries_len, "\"\n\0", sizeof("\"\n\0"));
						ini_entries_len += sizeof("\n\0\"") - 2;
					} else {
						cgi_sapi_module.ini_entries = realloc(cgi_sapi_module.ini_entries, ini_entries_len + len + sizeof("\n\0"));
						memcpy(cgi_sapi_module.ini_entries + ini_entries_len, php_optarg, len);
						memcpy(cgi_sapi_module.ini_entries + ini_entries_len + len, "\n\0", sizeof("\n\0"));
						ini_entries_len += len + sizeof("\n\0") - 2;
					}
				} else {
					cgi_sapi_module.ini_entries = realloc(cgi_sapi_module.ini_entries, ini_entries_len + len + sizeof("=1\n\0"));
					memcpy(cgi_sapi_module.ini_entries + ini_entries_len, php_optarg, len);
					memcpy(cgi_sapi_module.ini_entries + ini_entries_len + len, "=1\n\0", sizeof("=1\n\0"));
					ini_entries_len += len + sizeof("=1\n\0") - 2;
				}
				break;
			}

			case 'y':
				fpm_config = php_optarg;
				break;

			case 'e': /* enable extended info output */
				/* CG(extended_info) = 1; */ /* 5_2 */
				CG(compiler_options) |= ZEND_COMPILE_EXTENDED_INFO; /* 5_3 */
				break;

			case 'm': /* list compiled in modules */
				cgi_sapi_module.startup(&cgi_sapi_module);
				php_output_startup();
				php_output_activate(TSRMLS_C);
				SG(headers_sent) = 1;
				php_printf("[PHP Modules]\n");
				print_modules(TSRMLS_C);
				php_printf("\n[Zend Modules]\n");
				print_extensions(TSRMLS_C);
				php_printf("\n");
				php_end_ob_buffers(1 TSRMLS_CC);
				exit_status = 0;
				goto out;

			case 'i': /* php info & quit */
				cgi_sapi_module.startup(&cgi_sapi_module);
				if (php_request_startup(TSRMLS_C) == FAILURE) {
					SG(server_context) = NULL;
					php_module_shutdown(TSRMLS_C);
					return FAILURE;
				}
				SG(headers_sent) = 1;
				SG(request_info).no_headers = 1;
				php_print_info(0xFFFFFFFF TSRMLS_CC);
				php_request_shutdown((void *) 0);
				exit_status = 0;
				goto out;

			case 'h':
			case '?':
				cgi_sapi_module.startup(&cgi_sapi_module);
				php_output_startup();
				php_output_activate(TSRMLS_C);
				SG(headers_sent) = 1;
				php_cgi_usage(argv[0]);
				php_end_ob_buffers(1 TSRMLS_CC);
				exit_status = 0;
				goto out;

			case 'v': /* show php version & quit */
				cgi_sapi_module.startup(&cgi_sapi_module);
				if (php_request_startup(TSRMLS_C) == FAILURE) {
					SG(server_context) = NULL;
					php_module_shutdown(TSRMLS_C);
					return FAILURE;
				}
				SG(headers_sent) = 1;
				SG(request_info).no_headers = 1;

#if SUHOSIN_PATCH
#if ZEND_DEBUG
						php_printf("PHP %s with Suhosin-Patch %s (%s) (built: %s %s) (DEBUG)\nCopyright (c) 1997-2009 The PHP Group\n%s", PHP_VERSION, SUHOSIN_PATCH_VERSION, sapi_module.name, __DATE__, __TIME__, get_zend_version());
#else
						php_printf("PHP %s with Suhosin-Patch %s (%s) (built: %s %s)\nCopyright (c) 1997-2009 The PHP Group\n%s", PHP_VERSION, SUHOSIN_PATCH_VERSION, sapi_module.name, __DATE__, __TIME__, get_zend_version());
#endif
#else
#if ZEND_DEBUG
				php_printf("PHP %s (%s) (built: %s %s) (DEBUG)\nCopyright (c) 1997-2009 The PHP Group\n%s", PHP_VERSION, sapi_module.name, __DATE__, __TIME__, get_zend_version());
#else
				php_printf("PHP %s (%s) (built: %s %s)\nCopyright (c) 1997-2009 The PHP Group\n%s", PHP_VERSION, sapi_module.name, __DATE__, __TIME__, get_zend_version());
#endif
#endif
				php_request_shutdown((void *) 0);
				exit_status = 0;
				goto out;

		}

	}
	php_optind = orig_optind;
	php_optarg = orig_optarg;

#ifdef ZTS
	SG(request_info).path_translated = NULL;
#endif

	cgi_sapi_module.executable_location = argv[0];

	/* startup after we get the above ini override se we get things right */
	if (cgi_sapi_module.startup(&cgi_sapi_module) == FAILURE) {
#ifdef ZTS
		tsrm_shutdown();
#endif
		return FAILURE;
	}

	if (0 > fpm_init(argc, argv, fpm_config)) {
		return FAILURE;
	}

	fcgi_fd = fpm_run(&max_requests);

	parent = 0;

	fcgi_set_is_fastcgi(1);

	/* make php call us to get _ENV vars */
	php_php_import_environment_variables = php_import_environment_variables;
	php_import_environment_variables = cgi_php_import_environment_variables;

	/* library is already initialized, now init our request */
	fcgi_init_request(&request, fcgi_fd);

	zend_first_try {

		/* start of FAST CGI loop */
		/* Initialise FastCGI request structure */
#ifdef PHP_WIN32
		/* attempt to set security impersonation for fastcgi
		   will only happen on NT based OS, others will ignore it. */
		if (fastcgi && CGIG(impersonate)) {
			fcgi_impersonate();
		}
#endif
		while (fcgi_accept_request(&request) >= 0) {

		request_body_fd = -1;

		SG(server_context) = (void *) &request;

		init_request_info(TSRMLS_C);

		CG(interactive) = 0;

		fpm_request_info();

		/* 
			we never take stdin if we're (f)cgi, always
			rely on the web server giving us the info
			we need in the environment. 
		*/
		if (SG(request_info).path_translated) {
			file_handle.type = ZEND_HANDLE_FILENAME;
			file_handle.filename = SG(request_info).path_translated;
			file_handle.handle.fp = NULL;
		}
		file_handle.opened_path = NULL;
		file_handle.free_filename = 0;

		/* request startup only after we've done all we can to
		   get path_translated */
		if (php_request_startup(TSRMLS_C) == FAILURE) {
			fcgi_finish_request(&request);
			SG(server_context) = NULL;
			php_module_shutdown(TSRMLS_C);
			return FAILURE;
		}

		/* 
			at this point path_translated will be set if:
			1. we are running from shell and got filename was there
			2. we are running as cgi or fastcgi
		*/
		retval = FAILURE;
		if (SG(request_info).path_translated) {
			if (!php_check_open_basedir(SG(request_info).path_translated TSRMLS_CC)) {
				retval = php_fopen_primary_script(&file_handle TSRMLS_CC);
			}
		}
		/* 
			if we are unable to open path_translated and we are not
			running from shell (so fp == NULL), then fail.
		*/
		if (retval == FAILURE && file_handle.handle.fp == NULL) {
			if (errno == EACCES) {
				SG(sapi_headers).http_response_code = 403;
				PUTS("Access denied.\n");
			} else {
				SG(sapi_headers).http_response_code = 404;
				PUTS("No input file specified.\n");
			}
			/* we want to serve more requests if this is fastcgi
			   so cleanup and continue, request shutdown is
			   handled later */
			goto fastcgi_request_done;

			STR_FREE(SG(request_info).path_translated);

			if (free_query_string && SG(request_info).query_string) {
				free(SG(request_info).query_string);
				SG(request_info).query_string = NULL;
			}

			php_request_shutdown((void *) 0);
			SG(server_context) = NULL;
			php_module_shutdown(TSRMLS_C);
			sapi_shutdown();
#ifdef ZTS
			tsrm_shutdown();
#endif
			return FAILURE;
		}

		fpm_request_executing();

		php_execute_script(&file_handle TSRMLS_CC);

fastcgi_request_done:

		if (request_body_fd != -1) close(request_body_fd);

		request_body_fd = -2;

		{
			char *path_translated;

			/*	Go through this trouble so that the memory manager doesn't warn
			 *	about SG(request_info).path_translated leaking
			 */
			if (SG(request_info).path_translated) {
				path_translated = strdup(SG(request_info).path_translated);
				STR_FREE(SG(request_info).path_translated);
				SG(request_info).path_translated = path_translated;
			}

			if (EG(exit_status) == 255) {
				if (CGIG(error_header) && *CGIG(error_header)) {
					sapi_header_line ctr = {0};

					ctr.line = CGIG(error_header);
					ctr.line_len = strlen(CGIG(error_header));
					sapi_header_op(SAPI_HEADER_REPLACE, &ctr TSRMLS_CC);
				}
			}
			
			php_request_shutdown((void *) 0);
			if (exit_status == 0) {
				exit_status = EG(exit_status);
			}

			if (SG(request_info).path_translated) {
				free(SG(request_info).path_translated);
				SG(request_info).path_translated = NULL;
			}
			if (free_query_string && SG(request_info).query_string) {
				free(SG(request_info).query_string);
				SG(request_info).query_string = NULL;
			}

		}

		requests++;
		if (max_requests && (requests == max_requests)) {
			fcgi_finish_request(&request);
			break;
		}

		/* end of fastcgi loop */
		}

		fcgi_shutdown();

		if (fcgi_in_shutdown() || 								/* graceful shutdown by a signal */
				(max_requests && (requests == max_requests))	/* we were told to process max_requests and we are done */
			) {
			exit_status = 0;
		}
		else {
			exit_status = 255;
		}

		if (cgi_sapi_module.php_ini_path_override) {
			free(cgi_sapi_module.php_ini_path_override);
		}
		if (cgi_sapi_module.ini_entries) {
			free(cgi_sapi_module.ini_entries);
		}
	} zend_catch {
		exit_status = 255;
	} zend_end_try();

out:

	SG(server_context) = NULL;
	php_module_shutdown(TSRMLS_C);
	sapi_shutdown();

#ifdef ZTS
	/*tsrm_shutdown();*/
#endif

#if defined(PHP_WIN32) && ZEND_DEBUG && 0
	_CrtDumpMemoryLeaks();
#endif

	return exit_status;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
