
AC_DEFUN([AC_FPM_ARGS],
[
	PHP_ARG_WITH(fpm-bin,,
	[  --with-fpm-bin[=PATH]   Set the path for the php-fpm binary [/usr/local/bin/php-fpm]], /usr/local/bin/php-fpm, no)

	PHP_ARG_WITH(fpm-conf,,
	[  --with-fpm-conf[=PATH]  Set the path for php-fpm configuration file [/etc/php-fpm.conf]], /etc/php-fpm.conf, no)

	PHP_ARG_WITH(fpm-log,,
	[  --with-fpm-log[=PATH]   Set the path for php-fpm log file [var/log/php-fpm.log]], var/log/php-fpm.log, no)

	PHP_ARG_WITH(fpm-pid,,
	[  --with-fpm-pid[=PATH]   Set the path for php-fpm pid file [var/run/php-fpm.pid]], var/run/php-fpm.pid, no)

	PHP_ARG_WITH(fpm-user,,
	[  --with-fpm-user[=USER]  Set the user for php-fpm to run as [nobody]], nobody, no)

	PHP_ARG_WITH(fpm-group,,
	[  --with-fpm-group[=GRP]  Set the group for php-fpm to run as. For a system user, this 
	                          should usually be set to match the fpm username [nobody]], nobody, no)
])

AC_DEFUN([AC_FPM_VARS],
[
	if test -z "$PHP_FPM_BIN" -o "$with_fpm_bin" = "yes" -o "$with_fpm_bin" = "no"; then
		php_fpm_bin_path="$fpm_prefix/bin/php-fpm"
	else
		php_fpm_bin_path="$PHP_FPM_BIN"
	fi
	php_fpm_bin=`basename $php_fpm_bin_path`

	if test -z "$PHP_FPM_CONF" -o "$with_fpm_conf" = "yes" -o "$with_fpm_conf" = "no"; then
		php_fpm_conf_path="/etc/php-fpm.conf"
	else
		php_fpm_conf_path="$PHP_FPM_CONF"
	fi
	php_fpm_conf=`basename $php_fpm_conf_path`

	if test -z "$PHP_FPM_LOG" -o "$with_fpm_log" = "yes" -o "$with_fpm_log" = "no"; then
		php_fpm_log_path="/var/log/php-fpm.log"
	else
		php_fpm_log_path="$PHP_FPM_LOG"
	fi

	if test -z "$PHP_FPM_PID" -o "$with_fpm_pid" = "yes" -o "$with_fpm_pid" = "no"; then
		php_fpm_pid_path="/var/run/php-fpm.pid"
	else
		php_fpm_pid_path="$PHP_FPM_PID"
	fi

	if test -z "$PHP_FPM_USER" -o "$with_fpm_user" = "yes" -o "$with_fpm_user" = "no"; then
		php_fpm_user="nobody"
	else
		php_fpm_user="$PHP_FPM_USER"
	fi

	if test -z "$PHP_FPM_GROUP" -o "$with_fpm_group" = "yes" -o "$with_fpm_group" = "no"; then
		php_fpm_group="nobody"
	else
		php_fpm_group="$PHP_FPM_GROUP"
	fi


	PHP_SUBST_OLD(fpm_version)
	PHP_SUBST_OLD(php_fpm_bin)
	PHP_SUBST_OLD(php_fpm_bin_path)
	PHP_SUBST_OLD(php_fpm_conf_path)
	PHP_SUBST_OLD(php_fpm_log_path)
	PHP_SUBST_OLD(php_fpm_pid_path)
	PHP_SUBST_OLD(php_fpm_user)
	PHP_SUBST_OLD(php_fpm_group)


	AC_DEFINE_UNQUOTED(PHP_FPM_VERSION, "$fpm_version", [fpm version])
	AC_DEFINE_UNQUOTED(PHP_FPM_BIN, "$php_fpm_bin", [fpm binary executable])
	AC_DEFINE_UNQUOTED(PHP_FPM_BIN_PATH, "$php_fpm_bin_path", [fpm bin file path])
	AC_DEFINE_UNQUOTED(PHP_FPM_CONF_PATH, "$php_fpm_conf_path", [fpm conf file path])
	AC_DEFINE_UNQUOTED(PHP_FPM_LOG_PATH, "$php_fpm_log_path", [fpm log file path])
	AC_DEFINE_UNQUOTED(PHP_FPM_PID_PATH, "$php_fpm_pid_path", [fpm pid file path])
	AC_DEFINE_UNQUOTED(PHP_FPM_USER, "$php_fpm_user", [fpm user name])
	AC_DEFINE_UNQUOTED(PHP_FPM_GROUP, "$php_fpm_group", [fpm group name])

])


AC_DEFUN([AC_FPM_OUTPUT],
[
	PHP_OUTPUT(sapi/fpm/src/$php_fpm_conf:sapi/fpm/conf/php-fpm.conf.in)
	PHP_OUTPUT(sapi/fpm/src/init.d.$php_fpm_bin:sapi/fpm/conf/init.d.php-fpm.in)
	PHP_OUTPUT(sapi/fpm/src/nginx-site-conf.sample:sapi/fpm/conf/nginx-site-conf.sample.in)
	PHP_OUTPUT(sapi/fpm/src/$php_fpm_bin.1:sapi/fpm/man/php-fpm.1.in)
])


AC_DEFUN([AC_FPM_CONF],
[
	AC_FPM_ARGS
	AC_FPM_VARS
	AC_FPM_OUTPUT
])

