dnl @synopsis AC_LIB_EVENT([MINIMUM-VERSION])
dnl
dnl Test for the libevent library of a particular version (or newer).
dnl Source: http://svn.apache.org/repos/asf/incubator/thrift/trunk/aclocal/ax_lib_event.m4
dnl Modified: This file was modified for autoconf-2.13 and the PHP_ARG_WITH macro.
dnl
dnl If no path to the installed libevent is given, the macro will first try
dnl using no -I or -L flags, then searches under /usr, /usr/local, /opt,
dnl and /opt/libevent.
dnl If these all fail, it will try the $LIBEVENT_ROOT environment variable.
dnl
dnl This macro requires that #include <sys/types.h> works and defines u_char.
dnl
dnl This macro calls:
dnl   AC_SUBST(LIBEVENT_CFLAGS)
dnl   AC_SUBST(LIBEVENT_LIBS)
dnl
dnl And (if libevent is found):
dnl   AC_DEFINE(HAVE_LIBEVENT)
dnl
dnl It also leaves the shell variables "success" and "ac_have_libevent"
dnl set to "yes" or "no".
dnl
dnl NOTE: This macro does not currently work for cross-compiling,
dnl       but it can be easily modified to allow it.  (grep "cross").
dnl
dnl @category InstalledPackages
dnl @category C
dnl @version 2007-09-12
dnl @license AllPermissive
dnl
dnl Copyright (C) 2009 David Reiss
dnl Copying and distribution of this file, with or without modification,
dnl are permitted in any medium without royalty provided the copyright
dnl notice and this notice are preserved.

AC_DEFUN([AC_LIB_EVENT_DO_CHECK],
[
# Save our flags.
CPPFLAGS_SAVED="$CPPFLAGS"
LDFLAGS_SAVED="$LDFLAGS"
LIBS_SAVED="$LIBS"
LD_LIBRARY_PATH_SAVED="$LD_LIBRARY_PATH"

# Set our flags if we are checking a specific directory.
if test -n "$ac_libevent_path" ; then
	LIBEVENT_CPPFLAGS="-I$ac_libevent_path/include"
	LIBEVENT_LDFLAGS="-L$ac_libevent_path/lib"
	LD_LIBRARY_PATH="$ac_libevent_path/lib:$LD_LIBRARY_PATH"
else
	LIBEVENT_CPPFLAGS=""
	LIBEVENT_LDFLAGS=""
fi

# Required flag for libevent.
LIBEVENT_LIBS="-levent"

# Prepare the environment for compilation.
CPPFLAGS="$CPPFLAGS $LIBEVENT_CPPFLAGS"
LDFLAGS="$LDFLAGS $LIBEVENT_LDFLAGS"
LIBS="$LIBS $LIBEVENT_LIBS"
export CPPFLAGS
export LDFLAGS
export LIBS
export LD_LIBRARY_PATH

success=no

# Compile, link, and run the program.  This checks:
# - event.h is available for including.
# - event_get_version() is available for linking.
# - The event version string is lexicographically greater
#   than the required version.
AC_TRY_RUN([
#include <sys/types.h>
#include <event.h>

int main(int argc, char *argv[])
{
	const char* lib_version = event_get_version();
	const char* wnt_version = "$WANT_LIBEVENT_VERSION";
	for (;;) {
		/* If we reached the end of the want version.  We have it. */
		if (*wnt_version == '\0' || *wnt_version == '-') {
			return 0;
		}
		/* If the want version continues but the lib version does not, */
		/* we are missing a letter.  We don't have it. */
		if (*lib_version == '\0' || *lib_version == '-') {
			return 1;
		}

		/* In the 1.4 version numbering style, if there are more digits */
		/* in one version than the other, that one is higher. */
		int lib_digits;
		for (lib_digits = 0;
		lib_version[lib_digits] >= '0' &&
		lib_version[lib_digits] <= '9';
		lib_digits++)
		;
		int wnt_digits;
		for (wnt_digits = 0;
		wnt_version[wnt_digits] >= '0' &&
		wnt_version[wnt_digits] <= '9';
		wnt_digits++)
		;
		if (lib_digits > wnt_digits) {
			return 0;
		}
		if (lib_digits < wnt_digits) {
			return 1;
		}
		/* If we have greater than what we want.  We have it. */
		if (*lib_version > *wnt_version) {
			return 0;
		}
		/* If we have less, we don't. */
		if (*lib_version < *wnt_version) {
			return 1;
		}
		lib_version++;
		wnt_version++;
	}
	return 0;
}
],[
success=yes
])

# Restore flags.
LIBEVENT_LIBS=""
CPPFLAGS="$CPPFLAGS_SAVED"
LDFLAGS="$LDFLAGS_SAVED"
LIBS="$LIBS_SAVED"
LD_LIBRARY_PATH="$LD_LIBRARY_PATH_SAVED"
])

AC_DEFUN([AC_LIB_EVENT],
[

PHP_ARG_WITH(libevent,,
[  --with-libevent[=PATH]  Path to the libevent, needed for fpm SAPI [/usr/local]], yes, yes)

if test "$PHP_LIBEVENT" != "no"; then
	WANT_LIBEVENT_VERSION=ifelse([$1], ,1.2,$1)

	# Default library search paths ($sys_lib_search_path_spec)
	AC_LIBTOOL_SYS_DYNAMIC_LINKER

	AC_MSG_CHECKING(for libevent >= $WANT_LIBEVENT_VERSION)

	libevent_prefix=$ac_default_prefix
	if test $prefix != "NONE" -a $prefix != "" -a $prefix != "no" ; then 
		libevent_prefix=$prefix
	fi

    if test "$PHP_LIBEVENT" = "yes"; then
		PHP_LIBEVENT=$libevent_prefix
    fi

    for ac_libevent_path in "" $PHP_LIBEVENT /usr /usr/local /opt /opt/local /opt/libevent ; do
      AC_LIB_EVENT_DO_CHECK
      if test "$success" = "yes"; then
        break;
      fi
    done

	if test "$ext_shared" = "yes"; then
		if test -n "$ac_libevent_path"; then
			LIBEVENT_LIBS="-L$ac_libevent_path/lib -levent"
		else
			LIBEVENT_LIBS="-levent"
		fi
	else
		libevent_a="libevent.a"
		if test -n "$ac_libevent_path"; then
			if test -f "$ac_libevent_path/lib/$libevent_a" ; then
				LIBEVENT_LIBS="$ac_libevent_path/lib/$libevent_a"
			fi
		else
			for search_path in $sys_lib_search_path_spec ; do
				if test -f "$search_path$libevent_a" ; then
					LIBEVENT_LIBS="$search_path$libevent_a"
					break;
				fi
			done
		fi
		if test -z "$LIBEVENT_LIBS"; then
			AC_MSG_ERROR([libevent.a could not be found. Use --with-libevent=shared])
		fi
	fi

	if test "$success" != "yes" ; then
		AC_MSG_RESULT(no)
		ac_have_libevent=no
		AC_MSG_ERROR([Libevent $WANT_LIBEVENT_VERSION could not be found])
	else
		AC_MSG_RESULT(yes)
		ac_have_libevent=yes
		AC_DEFINE(HAVE_LIBEVENT, 1, [define if libevent is available])
	fi

	if test -n "$ac_libevent_path"; then
		LIBEVENT_CFLAGS="-I$ac_libevent_path/include"
	fi

    AC_SUBST(LIBEVENT_CFLAGS)
    AC_SUBST(LIBEVENT_LIBS)

else
	AC_MSG_ERROR([FPM Requires Libevent. You can build this target --with-libevent=yes])				
fi

])
