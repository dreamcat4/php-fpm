




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
ifneq ($(strip $(php_fpm_init)),)
	@echo "Installing PHP FPM init script:   $(INSTALL_ROOT)$(php_fpm_init_path)"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(php_fpm_init_dir)
	@$(INSTALL) -m 0755 init.d.$(php_fpm_init) $(INSTALL_ROOT)$(php_fpm_init_path)
@ENDIF@
	@echo ""
	@echo "*** FPM Installation complete. ***"
	@echo ""
ifneq ($(strip $(php_fpm_init)),)
	@echo "run:"
	@echo "\`update-rc.d $(php_fpm_init) defaults; invoke-rc.d $(php_fpm_init) start\`"
	@echo ""
	@echo "or system equivalent to start the $(php_fpm_init) service."
	@echo ""
@ENDIF@

