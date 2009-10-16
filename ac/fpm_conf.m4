
AC_DEFUN([AC_FPM_ARGS],
[
	PHP_ARG_WITH(fpm-bin,,
	[  --with-fpm-bin[=PATH]   Set the path for the php-fpm binary [/usr/local/bin/php-fpm]], yes, no)

	PHP_ARG_WITH(fpm-port,,
	[  --with-fpm-port[=PORT]  Set the tcp port number to listen for cgi requests [9000]], yes, no)

	PHP_ARG_WITH(fpm-conf,,
	[  --with-fpm-conf[=PATH]  Set the path for php-fpm configuration file [/etc/php-fpm.conf]], yes, no)

	PHP_ARG_WITH(fpm-init,,
	[  --with-fpm-init[=PATH]  Set the path for php-fpm init file [/etc/init.d/php-fpm]], yes, no)

	PHP_ARG_WITH(fpm-log,,
	[  --with-fpm-log[=PATH]   Set the path for php-fpm log file [/var/log/php-fpm.log]], yes, no)

	PHP_ARG_WITH(fpm-pid,,
	[  --with-fpm-pid[=PATH]   Set the path for php-fpm pid file [/var/run/php-fpm.pid]], yes, no)

	PHP_ARG_WITH(fpm-user,,
	[  --with-fpm-user[=USER]  Set the user for php-fpm to run as [nobody]], yes, no)

	PHP_ARG_WITH(fpm-group,,
	[  --with-fpm-group[=GRP]  Set the group for php-fpm to run as. For a system user,
	                  this should be set to match the fpm username [nobody]], yes, no)
])

AC_DEFUN([AC_FPM_VARS],
[
	fpm_prefix=$ac_default_prefix
	if test $prefix != "NONE" -a $prefix != "" -a $prefix != "no" ; then
		fpm_prefix=$prefix
	else
		prefix=$fpm_prefix
	fi

	if test $exec_prefix = "NONE" -o $exec_prefix = "" -o $exec_prefix = "no" ; then
		exec_prefix=$fpm_prefix
	fi

	if test `echo "$bindir" | grep "exec_prefix"` ; then
		bindir=$exec_prefix/bin
	fi

	fpm_bin_prefix=$fpm_prefix/bin
	if test $bindir != "NONE" -a $bindir != "" -a $bindir != "no" ; then
		fpm_bin_prefix=$bindir
	fi

	if test -z "$PHP_FPM_BIN" -o "$PHP_FPM_BIN" = "yes" -o "$PHP_FPM_BIN" = "no"; then
		php_fpm_bin_path="$fpm_bin_prefix/php-fpm"
	else
		php_fpm_bin_path="$PHP_FPM_BIN"
	fi
	php_fpm_bin=`basename $php_fpm_bin_path`
	php_fpm_bin_dir=`dirname $php_fpm_bin_path`

	if test -z "$PHP_FPM_PORT" -o "$PHP_FPM_PORT" = "yes" -o "$PHP_FPM_PORT" = "no"; then
		php_fpm_port="9000"
	else
		php_fpm_port="$PHP_FPM_PORT"
	fi

	if test -z "$PHP_FPM_CONF" -o "$PHP_FPM_CONF" = "yes"; then
		case $host_os in
			freebsd*|dragonfly*)  php_fpm_conf_path="/usr/local/etc/php-fpm.conf" ;;
			*)                    php_fpm_conf_path="/etc/php-fpm.conf" ;;
		esac
	elif test "$PHP_FPM_CONF" = "no"; then
		php_fpm_conf_path=""
	else
		php_fpm_conf_path="$PHP_FPM_CONF"
	fi
	if test -z "$with_fpm_conf_path"; then
		php_fpm_conf=""
		php_fpm_conf_dir=""
	else
		php_fpm_conf=`basename $php_fpm_conf_path`
		php_fpm_conf_dir=`dirname $php_fpm_conf_path`		
	fi

	if test -z "$PHP_FPM_INIT" -o "$PHP_FPM_INIT" = "yes"; then
		case $host_os in
			openbsd*)          php_fpm_init_path="" ;;
			netbsd*)           php_fpm_init_path="/etc/rc.d/php-fpm" ;;
			*bsd*|dragonfly*)  php_fpm_init_path="/usr/local/etc/rc.d/php-fpm" ;;
			*)                 php_fpm_init_path="/etc/init.d/php-fpm" ;;
		esac
		test -f /etc/arch-release && php_fpm_init_path="/etc/rc.d/php-fpm" # arch linux

	elif test "$PHP_FPM_INIT" = "no"; then
		php_fpm_init_path=""
	else
		php_fpm_init_path="$PHP_FPM_INIT"
	fi
	if test -z "$with_fpm_init_path"; then
		php_fpm_init=""
		php_fpm_init_dir=""
	else
		php_fpm_init=`basename $php_fpm_init_path`
		php_fpm_init_dir=`dirname $php_fpm_init_path`		
	fi

	if test -z "$PHP_FPM_LOG" -o "$PHP_FPM_LOG" = "yes" -o "$PHP_FPM_LOG" = "no"; then
		php_fpm_log_path="/var/log/php-fpm.log"
	else
		php_fpm_log_path="$PHP_FPM_LOG"
	fi
	php_fpm_log_dir=`dirname $php_fpm_log_path`

	if test -z "$PHP_FPM_PID" -o "$PHP_FPM_PID" = "yes" -o "$PHP_FPM_PID" = "no"; then
		php_fpm_pid_path="/var/run/php-fpm.pid"
	else
		php_fpm_pid_path="$PHP_FPM_PID"
	fi
	php_fpm_pid_dir=`dirname $php_fpm_pid_path`

	if test -z "$PHP_FPM_USER" -o "$PHP_FPM_USER" = "yes" -o "$PHP_FPM_USER" = "no"; then
		php_fpm_user="nobody"
	else
		php_fpm_user="$PHP_FPM_USER"
	fi

	if test -z "$PHP_FPM_GROUP" -o "$PHP_FPM_GROUP" = "yes" -o "$PHP_FPM_GROUP" = "no"; then
		php_fpm_group="nobody"
	else
		php_fpm_group="$PHP_FPM_GROUP"
	fi


	PHP_SUBST_OLD(fpm_version)
	PHP_SUBST_OLD(php_fpm_bin)
	PHP_SUBST_OLD(php_fpm_bin_dir)
	PHP_SUBST_OLD(php_fpm_bin_path)
	PHP_SUBST_OLD(php_fpm_port)
	PHP_SUBST_OLD(php_fpm_conf)
	PHP_SUBST_OLD(php_fpm_conf_dir)
	PHP_SUBST_OLD(php_fpm_conf_path)
	PHP_SUBST_OLD(php_fpm_init)
	PHP_SUBST_OLD(php_fpm_init_dir)
	PHP_SUBST_OLD(php_fpm_init_path)
	PHP_SUBST_OLD(php_fpm_log_dir)
	PHP_SUBST_OLD(php_fpm_log_path)
	PHP_SUBST_OLD(php_fpm_pid_dir)
	PHP_SUBST_OLD(php_fpm_pid_path)
	PHP_SUBST_OLD(php_fpm_user)
	PHP_SUBST_OLD(php_fpm_group)


	AC_DEFINE_UNQUOTED(PHP_FPM_VERSION, "$fpm_version", [fpm version])
	AC_DEFINE_UNQUOTED(PHP_FPM_BIN, "$php_fpm_bin", [fpm binary executable])
	AC_DEFINE_UNQUOTED(PHP_FPM_BIN_DIR, "$php_fpm_bin_dir", [fpm binary dir])
	AC_DEFINE_UNQUOTED(PHP_FPM_BIN_PATH, "$php_fpm_bin_path", [fpm bin file path])
	AC_DEFINE_UNQUOTED(PHP_FPM_PORT, "$php_fpm_port", [tcp port])
	AC_DEFINE_UNQUOTED(PHP_FPM_CONF, "$php_fpm_conf", [fpm conf file])
	AC_DEFINE_UNQUOTED(PHP_FPM_CONF_DIR, "$php_fpm_conf_dir", [fpm conf dir])
	AC_DEFINE_UNQUOTED(PHP_FPM_CONF_PATH, "$php_fpm_conf_path", [fpm conf file path])
	AC_DEFINE_UNQUOTED(PHP_FPM_INIT, "$php_fpm_init", [fpm init file])
	AC_DEFINE_UNQUOTED(PHP_FPM_INIT_DIR, "$php_fpm_init_dir", [fpm init dir])
	AC_DEFINE_UNQUOTED(PHP_FPM_INIT_PATH, "$php_fpm_init_path", [fpm init file path])
	AC_DEFINE_UNQUOTED(PHP_FPM_LOG_DIR, "$php_fpm_log_dir", [fpm log dir])
	AC_DEFINE_UNQUOTED(PHP_FPM_LOG_PATH, "$php_fpm_log_path", [fpm log file path])
	AC_DEFINE_UNQUOTED(PHP_FPM_PID_DIR, "$php_fpm_pid_dir", [fpm pid dir])
	AC_DEFINE_UNQUOTED(PHP_FPM_PID_PATH, "$php_fpm_pid_path", [fpm pid file path])
	AC_DEFINE_UNQUOTED(PHP_FPM_USER, "$php_fpm_user", [fpm user name])
	AC_DEFINE_UNQUOTED(PHP_FPM_GROUP, "$php_fpm_group", [fpm group name])

])


AC_DEFUN([AC_FPM_OUTPUT],
[
	PHP_OUTPUT(sapi/fpm/$php_fpm_conf:sapi/fpm/conf/php-fpm.conf.in)
	PHP_OUTPUT(sapi/fpm/init.d.$php_fpm_init:sapi/fpm/conf/init.d.php-fpm.in)
	PHP_OUTPUT(sapi/fpm/nginx-site-conf.sample:sapi/fpm/conf/nginx-site-conf.sample.in)
	PHP_OUTPUT(sapi/fpm/$php_fpm_bin.1:sapi/fpm/man/php-fpm.1.in)
])


AC_DEFUN([AC_FPM_CONF],
[
	AC_FPM_ARGS
	AC_FPM_VARS
	AC_FPM_OUTPUT
])

