
SUBDIRS = cgi fpm

bindir = $(php_fpm_bin_dir)
bin_PROGRAMS = $(php_fpm_bin)

$(php_fpm_bin): php_fpm
	ln -sf php_fpm $(php_fpm_bin)

EXTRA_PROGRAMS = php_fpm
php_fpm_SOURCES =
php_fpm_LDFLAGS =-export-dynamic

php_fpm_LDADD = \
	$(PHP_OBJS:%=$(PHP_BUILDDIR)/%) \
	cgi/libsapi.la \
	fpm/libfpm.la \
	$(php_fpm_ADD_LIBS)
