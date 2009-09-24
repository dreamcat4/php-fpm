
AC_DEFUN([AC_FPM_BUILD_SAPI],
[
	PHP_ADD_MAKEFILE_FRAGMENT($abs_srcdir/sapi/fpm/ac/Makefile.frag,$abs_srcdir/sapi/fpm,sapi/fpm)

    SAPI_FPM_PATH=sapi/fpm/$php_fpm_bin
	PHP_SUBST(SAPI_FPM_PATH)
	
	mkdir -p sapi/fpm/cgi
	PHP_FPM_SAPI_FILES=`cd $abs_srcdir/sapi/fpm && find cgi/ \( -name *.c \) -exec printf "{} " \;`
	# PHP_FPM_SAPI_FILES="cgi/cgi_main.c cgi/fastcgi.c cgi/getopt.c"

	mkdir -p sapi/fpm/fpm
	PHP_FPM_CORE_FILES=`cd $abs_srcdir/sapi/fpm && find fpm/ \( -name *.c -not -name fpm_trace*.c \) -exec printf "{} " \;`
	# PHP_FPM_CORE_FILES="fpm/fpm_process_ctl.c fpm/fpm_signals.c fpm/fpm_shm.c fpm/fpm.c fpm/fpm_worker_pool.c fpm/fpm_clock.c fpm/fpm_env.c fpm/fpm_shm_slots.c fpm/fpm_children.c fpm/fpm_events.c fpm/fpm_php.c fpm/fpm_unix.c fpm/fpm_request.c fpm/fpm_sockets.c fpm/fpm_php_trace.c fpm/zlog.c fpm/fpm_cleanup.c fpm/fpm_conf.c fpm/xml_config.c fpm/fpm_stdio.c"

	if test "$fpm_trace_type" ; then
		PHP_FPM_TRACE_FILES=`cd $abs_srcdir/sapi/fpm && find fpm/ \( -name fpm_trace.c -or -name fpm_trace_$fpm_trace_type.c \) -exec printf "{} " \;`
	fi
	
	PHP_FPM_CFLAGS="$LIBEVENT_CFLAGS -I$abs_srcdir/sapi/fpm"

	SAPI_EXTRA_LIBS="$LIBEVENT_LIBS"
	PHP_SUBST(SAPI_EXTRA_LIBS)
	
    dnl Set install target and select SAPI
	INSTALL_IT=""

    PHP_SELECT_SAPI(fpm, program, $PHP_FPM_SAPI_FILES $PHP_FPM_CORE_FILES $PHP_FPM_TRACE_FILES, $PHP_FPM_CFLAGS, '$(SAPI_FPM_PATH)')

    case $host_alias in
      *aix*)
        BUILD_FPM="echo '\#! .' > php.sym && echo >>php.sym && nm -BCpg \`echo \$(PHP_GLOBAL_OBJS) \$(PHP_SAPI_OBJS) | sed 's/\([A-Za-z0-9_]*\)\.lo/\1.o/g'\` | \$(AWK) '{ if (((\$\$2 == \"T\") || (\$\$2 == \"D\") || (\$\$2 == \"B\")) && (substr(\$\$3,1,1) != \".\")) { print \$\$3 } }' | sort -u >> php.sym && \$(LIBTOOL) --mode=link \$(CC) -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) -Wl,-brtl -Wl,-bE:php.sym \$(PHP_RPATHS) \$(PHP_GLOBAL_OBJS) \$(PHP_SAPI_OBJS) \$(EXTRA_LIBS) \$(SAPI_EXTRA_LIBS) \$(ZEND_EXTRA_LIBS) -o \$(SAPI_FPM_PATH)"
        ;;
      *darwin*)
        BUILD_FPM="\$(CC) \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(NATIVE_RPATHS) \$(PHP_GLOBAL_OBJS:.lo=.o) \$(PHP_SAPI_OBJS:.lo=.o) \$(PHP_FRAMEWORKS) \$(EXTRA_LIBS) \$(SAPI_EXTRA_LIBS) \$(ZEND_EXTRA_LIBS) -o \$(SAPI_FPM_PATH)"
      ;;
      *)
        BUILD_FPM="\$(LIBTOOL) --mode=link \$(CC) -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(PHP_RPATHS) \$(PHP_GLOBAL_OBJS) \$(PHP_SAPI_OBJS) \$(EXTRA_LIBS) \$(SAPI_EXTRA_LIBS) \$(ZEND_EXTRA_LIBS) -o \$(SAPI_FPM_PATH)"
      ;;
    esac

    PHP_SUBST(BUILD_FPM)

])
