
AC_DEFUN([AC_FPM_BUILD_SAPI],
[
	PHP_ADD_MAKEFILE_FRAGMENT($abs_srcdir/sapi/fpm/Makefile.frag)

    SAPI_FPM_PATH=sapi/fpm/$php_fpm_bin
	PHP_SUBST(SAPI_FPM_PATH)
	
	mkdir -p sapi/fpm/src/sapi
	PHP_FPM_SAPI_FILES=`cd $abs_srcdir/sapi/fpm && find src/sapi \( -name *.c \) -exec printf "{} " \;`
	# PHP_FPM_SAPI_FILES="src/sapi/cgi_main.c src/sapi/fastcgi.c src/sapi/getopt.c"

	mkdir -p sapi/fpm/src/fpm
	PHP_FPM_CORE_FILES=`cd $abs_srcdir/sapi/fpm && find src/fpm \( -name *.c -not -name fpm_trace*.c \) -exec printf "{} " \;`
	# PHP_FPM_CORE_FILES="src/fpm/fpm_process_ctl.c src/fpm/fpm_signals.c src/fpm/fpm_shm.c src/fpm/fpm.c src/fpm/fpm_worker_pool.c src/fpm/fpm_clock.c src/fpm/fpm_env.c src/fpm/fpm_shm_slots.c src/fpm/fpm_children.c src/fpm/fpm_events.c src/fpm/fpm_php.c src/fpm/fpm_unix.c src/fpm/fpm_request.c src/fpm/fpm_sockets.c src/fpm/fpm_php_trace.c src/fpm/zlog.c src/fpm/fpm_cleanup.c src/fpm/fpm_conf.c src/fpm/xml_config.c src/fpm/fpm_stdio.c"

	if test "$fpm_trace_type" ; then
		PHP_FPM_TRACE_FILES=`cd $abs_srcdir/sapi/fpm && find src/fpm \( -name fpm_trace.c -or -name fpm_trace_$fpm_trace_type.c \) -exec printf "{} " \;`
	fi
	
	PHP_FPM_CFLAGS="$LIBEVENT_CFLAGS -I$abs_srcdir/sapi/fpm/src"
	# PHP_FPM_CFLAGS="$LIBEVENT_CFLAGS"

	SAPI_EXTRA_LIBS="$LIBEVENT_LIBS"
	PHP_SUBST(SAPI_EXTRA_LIBS)
	
    dnl Set install target and select SAPI
    INSTALL_IT="@echo \"Installing PHP FPM binary: \$(INSTALL_ROOT)\$(bindir)/\"; \$(INSTALL) -m 0755 \$(SAPI_FPM_PATH) \$(INSTALL_ROOT)\$(bindir)/\$(program_prefix)php-fpm\$(program_suffix)\$(EXEEXT)"
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


