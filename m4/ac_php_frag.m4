dnl
dnl $Id$
dnl
dnl This file contains local autoconf functions.
dnl

AC_DEFUN([PHP_ARG_ANALYZE_EX],[
ext_output="yes, shared"
ext_shared=yes
case [$]$1 in
shared,*[)]
  $1=`echo "[$]$1"|sed 's/^shared,//'`
  ;;
shared[)]
  $1=yes
  ;;
no[)]
  ext_output=no
  ext_shared=no
  ;;
*[)]
  ext_output=yes
  ext_shared=no
  ;;
esac

])

AC_DEFUN([PHP_ARG_ANALYZE],[
ifelse([$3],yes,[PHP_ARG_ANALYZE_EX([$1])],[ext_output=ifelse([$]$1,,no,[$]$1)])
ifelse([$2],,,[AC_MSG_RESULT([$ext_output])])
])

dnl
dnl PHP_ARG_WITH(arg-name, check message, help text[, default-val[, extension-or-not]])
dnl Sets PHP_ARG_NAME either to the user value or to the default value.
dnl default-val defaults to no.  This will also set the variable ext_shared,
dnl and will overwrite any previous variable of that name.
dnl If extension-or-not is yes (default), then do the ENABLE_ALL check and run
dnl the PHP_ARG_ANALYZE_EX.
dnl
AC_DEFUN([PHP_ARG_WITH],[
AS_REAL_ARG_WITH([$1],[$2],[$3],[$4],PHP_[]translit($1,a-z0-9-,A-Z0-9_),[ifelse($5,,yes,$5)])
])

AC_DEFUN([AS_REAL_ARG_WITH],[
ifelse([$2],,,[AC_MSG_CHECKING([$2])])
AC_ARG_WITH($1,[$3],$5=[$]withval,
[
  $5=ifelse($4,,no,$4)

])
PHP_ARG_ANALYZE($5,[$2],$6)
])
