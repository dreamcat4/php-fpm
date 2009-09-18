# PHP FastCGI Process Manager

[Php fpm](http://php-fpm.org/Main_Page) is a standalone process manager and extra enhancements for PHP running via FastCGI. Source code at [Launchpad](https://launchpad.net/php-fpm)

## Quick steps to get started:

There are (1) Dependencies. If you haven't built php before, you'll need to install libxml dev package. The command for debian / ubuntu is:

	sudo aptitude install -y libxml2-dev

The PHP source code. Download it, unpack it, compile it.

	wget http://us.php.net/get/php-5.2.10.tar.gz/from/us.php.net/mirror
	tar xvfz php-5.2.10.tar.gz
	cd php-5.2.10
	./configure
	make

Check out php-fpm from launchpad

	bzr branch lp:~dreamcat4/php-fpm/master php-fpm

Configure and compile the new php-fpm

	cd php-fpm
	./configure \
	  --with-fpm-bin=/usr/bin/php-fpm \
	  --with-fpm-pid=/var/run/php-fpm.pid \
	  --with-fpm-log=/var/log/php-fpm.log \
	  --with-fpm-conf=/etc/php-fpm.conf \
	  --with-php-src=../

	make

Optional configure flags

	# Specify when build-dir is different to src-dir
	--with-php-build=[DIR]
	
	# Should be set to 'www-data' for debian based systems
	--with-fpm-user=[nobody]
	--with-fpm-group=[nobody]

## Autoconf

This project relies upon its own versions of the autoconf toolset to generate its `./configure` script. If you need to use autoconf, then run `./build-autotools` to install it locally. If `./build-autotools` fails please consult autoconf.markdown for more information.

## Build process

The make process can be described as:

	 1) Compile the php sources into object files in the php build directory
	 2) Compile the fpm sources into object files in the fpm build directory
	 3) Link all the php object file with these fpm object file together
	 4) Output: Static php5 binary, which is php base and using the fpm's version of fcgi-SAPI as frontend

Fpm is mixed into php at the link-level. This de-couples the fpm sources, making the process manager part somewhat less sensitive to changes in the php project. PHP-FPM is derived from the fcgi-sapi. We no longer patch directly onto php-maintained files. Instead there are 3 similar counterpart files from sapi/cgi and fpm's sapi are periodically synced to them.
