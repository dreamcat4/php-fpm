




install: install-fpm

install-fpm: $(php_fpm_bin)
	@echo "Installing PHP FPM binary:        $(INSTALL_ROOT)$(php_fpm_bin_path)"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(php_fpm_bin_dir)
	@$(mkinstalldirs) $(INSTALL_ROOT)$(php_fpm_pid_dir)
	@$(mkinstalldirs) $(INSTALL_ROOT)$(php_fpm_log_dir)
	@$(INSTALL) -m 0755 $(php_fpm_bin) $(INSTALL_ROOT)$(php_fpm_bin_path)$(program_suffix)$(EXEEXT)

	@test "$(php_fpm_conf)" && \
	@echo "Installing PHP FPM config:        $(INSTALL_ROOT)$(php_fpm_conf_path)" && \
	$(mkinstalldirs) $(INSTALL_ROOT)$(php_fpm_conf_dir)

	@test "$(php_fpm_conf)" && \
	test -f "$(INSTALL_ROOT)$(php_fpm_conf_path)" && \
	$(INSTALL_DATA) $(INSTALL_ROOT)$(php_fpm_conf_path) $(INSTALL_ROOT)$(php_fpm_conf_path).old

	@test "$(php_fpm_conf)" && \
	$(INSTALL_DATA) php_fpm.conf $(INSTALL_ROOT)$(php_fpm_conf_path).default && \
	ln -sf $(INSTALL_ROOT)$(php_fpm_conf_path).default $(INSTALL_ROOT)$(php_fpm_conf_path)

	@echo "Installing PHP FPM man page:      $(INSTALL_ROOT)$(mandir)/man1/$(php_fpm_bin)$(program_suffix).1"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(mandir)/man1
	@$(INSTALL_DATA) $(php_fpm_bin).1 $(INSTALL_ROOT)$(mandir)/man1/$(php_fpm_bin)$(program_suffix).1

	@test "$(php_fpm_init)" && \
	echo "Installing PHP FPM init script:   $(INSTALL_ROOT)$(php_fpm_init_path)" && \
	$(mkinstalldirs) $(INSTALL_ROOT)$(php_fpm_init_dir) && \
	$(INSTALL) -m 0755 init.d.php_fpm $(INSTALL_ROOT)$(php_fpm_init_path)

	@test -d /etc/nginx/ && \
	echo "Installing NGINX sample config:   /etc/nginx/nginx-site-conf.sample" && \
	@$(INSTALL_DATA) -b nginx-site-conf.sample /etc/nginx/nginx-site-conf.sample

	@test -d /usr/local/etc/nginx/ && \
	echo "Installing NGINX sample config:   /usr/local/etc/nginx/nginx-site-conf.sample" && \
	@$(INSTALL_DATA) -b nginx-site-conf.sample /usr/local/etc/nginx/nginx-site-conf.sample

	@test -d /usr/local/nginx/conf/ && \
	echo "Installing NGINX sample config:   /usr/local/nginx/conf/nginx-site-conf.sample" && \
	@$(INSTALL_DATA) -b nginx-site-conf.sample /usr/local/nginx/conf/nginx-site-conf.sample

	@echo ""
	@echo "*** FPM Installation complete. ***"
	@echo ""

	@test "$(php_fpm_init)" && \
	echo "run:" && \
	echo "\`update-rc.d $(php_fpm_init) defaults; invoke-rc.d $(php_fpm_init) start\`" && \
	echo "" && \
	echo "or system equivalent to start the $(php_fpm_init) service." && \
	echo ""

