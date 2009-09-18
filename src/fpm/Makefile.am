
INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(PHP_SRCDIR) \
	-I$(PHP_SRCDIR)/TSRM \
	-I$(PHP_SRCDIR)/Zend \
	-I$(PHP_SRCDIR)/main \
	-I$(PHP_BUILDDIR)/TSRM \
	-I$(PHP_BUILDDIR)/Zend \
	-I$(PHP_BUILDDIR)/main

noinst_LTLIBRARIES = libfpm.la

libfpm_la_CFLAGS = -DFPM_AUTOCONFIG_H $(php_fpm_ADD_CFLAGS)

libfpm_la_SOURCES = \
	fpm.c \
	fpm_conf.c \
	fpm_signals.c \
	fpm_children.c \
	fpm_worker_pool.c \
	fpm_unix.c \
	fpm_cleanup.c \
	fpm_sockets.c \
	fpm_stdio.c \
	fpm_env.c \
	fpm_events.c \
	fpm_php.c \
	fpm_php_trace.c \
	fpm_process_ctl.c \
	fpm_request.c \
	fpm_clock.c \
	fpm_shm.c \
	fpm_shm_slots.c \
	xml_config.c \
	zlog.c

if FPM_TRACE_USE_PTRACE
libfpm_la_SOURCES += fpm_trace.c fpm_trace_ptrace.c
endif

if FPM_TRACE_USE_PREAD
libfpm_la_SOURCES += fpm_trace.c fpm_trace_pread.c
endif

if FPM_TRACE_USE_MACH
libfpm_la_SOURCES += fpm_trace.c fpm_trace_mach.c
endif

