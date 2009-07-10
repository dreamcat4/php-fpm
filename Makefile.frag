
install-fpm: sapi/cgi/fpm/php-fpm.conf sapi/cgi/fpm/php-fpm
	@echo "Installing FPM config:            $(INSTALL_ROOT)$(php_fpm_conf_path)"
	-@$(mkinstalldirs) \
		$(INSTALL_ROOT)$(prefix)/sbin \
		`dirname "$(INSTALL_ROOT)$(php_fpm_conf_path)"` \
		`dirname "$(INSTALL_ROOT)$(php_fpm_log_path)"` \
		`dirname "$(INSTALL_ROOT)$(php_fpm_pid_path)"`
	-@if test -r "$(INSTALL_ROOT)$(php_fpm_conf_path)" ; then \
		dest=`basename "$(php_fpm_conf_path)"`.default ; \
		echo "                                  (installing as $$dest)" ; \
	else \
		dest=`basename "$(php_fpm_conf_path)"` ; \
	fi ; \
	$(INSTALL_DATA) $(top_builddir)/sapi/cgi/fpm/php-fpm.conf $(INSTALL_ROOT)`dirname "$(php_fpm_conf_path)"`/$$dest
	@echo "Installing init.d script:         $(INSTALL_ROOT)$(prefix)/sbin/php-fpm"
	-@$(INSTALL) -m 0755 $(top_builddir)/sapi/cgi/fpm/php-fpm $(INSTALL_ROOT)$(prefix)/sbin/php-fpm

$(top_builddir)/libevent/libevent.a: $(top_builddir)/libevent/Makefile
	cd $(top_builddir)/libevent && $(MAKE) libevent.a

