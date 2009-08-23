
SUBDIRS = sapi fpm

bin_PROGRAMS = $(PHP_FPM_BIN)
AM_CFLAGS +=-export-dynamic

$(PHP_FPM_BIN): php_fpm
	ln -sf php_fpm $(PHP_FPM_BIN)

EXTRA_PROGRAMS = php_fpm
php_fpm_SOURCES =

php_fpm_LDADD = \
	$(PHP_OBJS:%=$(PHP_BUILDDIR)/%) \
	sapi/libsapi.la \
	fpm/libfpm.la \
	../libevent/libevent.la \
	$(php_fpm_ADD_LIBS)

BUILT_SOURCES = ../libevent/event-config.h

../libevent/libevent.la:	../libevent/Makefile
	cd ../libevent && $(MAKE) libevent.la

../libevent/event-config.h:	../libevent/Makefile
	cd ../libevent && $(MAKE) event-config.h
