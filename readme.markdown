# PHP FastCGI Process Manager - http://php-fpm.org/Main_Page

Standalone process manager and extra enhancements for PHP running via FastCGI.
https://launchpad.net/php-fpm


## Quick steps to get started:

There are (1) Dependencies. If you haven't build php before, you'll need to install libxml dev package. The command for debian / ubuntu is:

	sudo aptitude install -y libxml2-dev

Download and unpack vanilla PHP, compile it. Currently 5.2.10 is tested, nothing else guaranteed yet.

	wget http://us.php.net/get/php-5.2.10.tar.gz/from/us.php.net/mirror
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

	# Specify when build-dir is different to src-dir
	--with-php-build=[DIR]

## Build process

The make process can be described as:

 1) Compile the php sources into object files in the php build directory
 2) Compile the fpm sources into object files in the fpm build directory
 3) Link each fpm object file to its corresponding (same name) php object file
 4) Output: Static php5 binary, which is php base and including the fpm-cgi extensions

Fpm is mixed in with php at the link-level. This de-couples the fpm sources, making them somewhat less sensitive to small changes in the php source code. We no longer are patching directly onto php source files, but there are 'corresponding' or counterpart source files in those part that fpm touches.

## Installation (untested)

	make install-fpm

