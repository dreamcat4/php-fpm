# PHP FastCGI Process Manager - http://php-fpm.org/Main_Page

Standalone process manager and extra enhancements for PHP running via FastCGI.
https://launchpad.net/php-fpm


## Quick steps to get started:

Download and unpack vanilla PHP, compile it. Currently 5.2.10 is tested, nothing else guaranteed yet.

	tar xvfz php-5.2.10.tar.gz
	cd php-5.2.10
	./configure
	make
	cd ..

Check out from launchpad

	bzr co lp:php-fpm php-fpm

Configure and compile the new php-fpm

	cd php-fpm
	./configure \
	  --with-fpm-pid=/var/run/php-fpm.pid \
	  --with-fpm-log=/var/log/php-fpm.log \
	  --with-fpm-conf=/etc/php-fpm.conf \
	  --with-php-src=../php-5.2.10

	make

Optional configure flags

	--with-php-build=[DIR]    # If different to php source dir

## Installation (untested)

	make install-fpm