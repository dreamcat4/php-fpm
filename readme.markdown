# PHP FastCGI Process Manager

PHP-FPM is a Fast-CGI frontend for php, and an enhancement of php-cgi. The project is hosted on [Launchpad](https://launchpad.net/php-fpm)

## Quick start:

Choose one of 2 ways to compile fpm. Either `integrated` or `seperate`. Unless you are a developer or systems administrator, we recommend the `integrated` compilation approach. For more information see the file `readme.markdown`.

## Integrated compilation

Download fpm and generate the patch file

	export PHP_VER=5.2.10 
	wget http://launchpad.net/php-fpm/trunk/0.6/+download/php-fpm-0.6-$PHP_VER.tar.gz
	php-fpm-0.6-$PHP_VER/generate-fpm-patch

The PHP source code. Download it, unpack it.

	wget http://us.php.net/get/php-$PHP_VER.tar.gz/from/us.php.net/mirror
	tar xvfz php-$PHP_VER.tar.gz
	cd php-$PHP_VER

Apply the patch and compile

	patch -p1 < ../fpm.patch
	./buildconf --force
	mkdir fpm-build && cd fpm-build
	../configure --with-fpm && make

## Seperate compilation
There are (2) Dependencies. If you haven't built php before, you'll need to install libxml dev package. FPM also requires the Libevent library. So install libevent-dev too. The command for debian / ubuntu is:

	sudo aptitude install -y libxml2-dev libevent-dev

The PHP source code. Download it, unpack it, compile it.

	export PHP_VER=5.2.10
	wget http://us.php.net/get/php-$PHP_VER.tar.gz/from/us.php.net/mirror
	tar xvfz php-$PHP_VER.tar.gz
	cd php-$PHP_VER
	mkdir php-build && cd php-build
	../configure && make

Now you can download, Configure and compile the FPM frontend

	wget http://launchpad.net/php-fpm/trunk/0.6/+download/php-fpm-0.6-$PHP_VER.tar.gz
	tar -zxvf php-fpm-0.6-$PHP_VER.tar.gz
	cd php-fpm-0.6-$PHP_VER
	mkdir fpm-build && cd fpm-build
	../configure --srcdir=../ \
	 --with-php-src=../../php-$PHP_VER \
	 --with-php-build=../../php-$PHP_VER/php-build \
	&& make

## Configure flags

	--with-libevent[=PATH]  Path to the libevent, for fpm SAPI [/usr/local]
	--with-fpm-bin[=PATH]   Set the path for the php-fpm binary [/usr/local/bin/php-fpm]
	--with-fpm-port[=PORT]  Set the tcp port number to listen for cgi requests [9000]
	--with-fpm-conf[=PATH]  Set the path for the php-fpm configuration file [/etc/php-fpm.conf]
	--with-fpm-log[=PATH]   Set the path for the php-fpm log file [/var/log/php-fpm.log]
	--with-fpm-pid[=PATH]   Set the path for the php-fpm pid file [/var/run/php-fpm.pid]
	--with-fpm-user[=USER]  Set the user for the php-fpm to run as [nobody]
	--with-fpm-group[=GRP]  Set the group for php-fpm to run as. For a system user,
		                  	this should be set to match the fpm username [nobody]

## Installation

If compiled `integrated`, then you will get a full php install, including the command line interpreter `php-cli`, and the PEAR extensions library. The `seperate` or `standalone` compilation method will install only the `php-fpm` daemon and the bare minimum of files required to run it.

	# See which files will be installed
	make install --dry-run

	# Install into '/'
	sudo make install

	# Install into '/opt'
	sudo INSTALL_ROOT=/opt make install

## More about the PHP-FPM Build process

The make process can be described as:

	 1) Compile the php sources into object files in the php build directory
	 2) Compile the fpm sources into object files in the fpm build directory
	 3) Link all the php object file with these fpm object file together
	 4) Output: Static php5 binary, which is php as base and fpm's fast-CGI SAPI as frontend

Fpm is mixed into php at the link-level. Andrei has de-coupled the fpm sources, making the SAPI part somewhat less sensitive to changes in the rest of php. PHP-FPM cgi-main.c request handler is derived from the original fcgi-sapi. We are submitting this 0.6 code to PHP Group. We will track developments in PHP and periodically sync changes back from them into the external / standalone project.

## Buildconf

For the external fpm project, generating `./configure` requires certiain versions of the autoconf toolset. You need at least some version of autoconf to install these build tools. If you need to use run buildconf, then it will run `./generate-autotools` and try to install these locally. If `./buildconf` fails please consult the error log.


## Mailinglist

There are 2 discussion groups for php-fpm,

- [highload-php-ru](http://groups.google.com/group/highload-php-en) (english)

- [highload-php-ru](http://groups.google.com/group/highload-php-ru) (Russian)





