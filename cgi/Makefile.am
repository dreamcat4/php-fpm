
INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(PHP_SRCDIR) \
	-I$(PHP_SRCDIR)/TSRM \
	-I$(PHP_SRCDIR)/Zend \
	-I$(PHP_SRCDIR)/main \
	-I$(PHP_BUILDDIR)/TSRM \
	-I$(PHP_BUILDDIR)/Zend \
	-I$(PHP_BUILDDIR)/main

noinst_LTLIBRARIES = libsapi.la

libsapi_la_CFLAGS = -DFPM_AUTOCONFIG_H

libsapi_la_SOURCES = \
	cgi_main.c \
	fastcgi.c \
	getopt.c

