dnl
dnl $Id$
dnl

PHP_ARG_ENABLE(fpm,,
[  --with-fpm              EXPERIMENTAL: Enable building of the fpm SAPI executable], yes, no)

# if test "$PHP_SAPI" = "default"; then
#   AC_MSG_CHECKING(whether to build FPM binary)

	if test "$PHP_FPM" != "no"; then
	    # AC_MSG_RESULT(yes)
	
		PHP_CONFIGURE_PART(Configuring fpm)
		
		sinclude(sapi/fpm/ac/fpm_libevent.m4)
			AC_LIB_EVENT([1.4.11])
		
		sinclude(sapi/fpm/ac/fpm_checks.m4)
			AC_FPM_CHECKS
		
		sinclude(sapi/fpm/ac/fpm_conf.m4)
			fpm_version="0.6.5"
			AC_FPM_CONF

		sinclude(sapi/fpm/ac/fpm_build.m4)
			AC_FPM_BUILD_SAPI

		AC_MSG_RESULT()
	fi

# fi
