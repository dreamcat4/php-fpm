
install: install-fpm

install-fpm: $(php_fpm_bin)
	@echo "Installing PHP FPM binary:        $(INSTALL_ROOT)$(php_fpm_bin_path)"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(php_fpm_bin_dir)
	@$(mkinstalldirs) $(INSTALL_ROOT)$(php_fpm_pid_dir)
	@$(mkinstalldirs) $(INSTALL_ROOT)$(php_fpm_log_dir)
	@$(INSTALL) -m 0755 $(php_fpm_bin) $(INSTALL_ROOT)$(php_fpm_bin_path)$(program_suffix)$(EXEEXT)
	@echo "Installing PHP FPM config:        $(INSTALL_ROOT)$(php_fpm_conf_path)"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(php_fpm_conf_dir)
	@$(INSTALL_DATA) $(php_fpm_conf) $(INSTALL_ROOT)$(php_fpm_conf_path)
	@echo "Installing PHP FPM man page:      $(INSTALL_ROOT)$(mandir)/man1/$(php_fpm_bin)$(program_suffix).1"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(mandir)/man1
	@$(INSTALL_DATA) $(php_fpm_bin).1 $(INSTALL_ROOT)$(mandir)/man1/$(php_fpm_bin)$(program_suffix).1
	@echo "Installing PHP FPM init script:   $(INSTALL_ROOT)/etc/init.d/$(php_fpm_bin)"
	@$(mkinstalldirs) $(INSTALL_ROOT)/etc/init.d
	@$(INSTALL_DATA) init.d.$(php_fpm_bin) $(INSTALL_ROOT)/etc/init.d/$(php_fpm_bin)


