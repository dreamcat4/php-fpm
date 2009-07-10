
SUBDIRS = sapi fpm

bin_PROGRAMS = php-fpm

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

