
## Autoconf - an introduction

Autoconf relies upon a number of staged (chained-together) files. Input files are autogenerated from templates, some are edited whilst others are left untouched. The relationships are somewhat complex. For a better understanding of the role of these files consult:

[Linux Self Help - Autoconf overview](http://www.linuxselfhelp.com/gnu/autoconf/html_chapter/autoconf_3.html), 

[Autoconf Book - the configure process](http://sourceware.org/autobook/autobook/autobook_279.html#SEC279)

[Autoconf Book - acinclude.m4 input file](http://sourceware.org/autobook/autobook/autobook_69.html#SEC69)

## Buildconf

The `buildconf` script is used to re-generate `configure` from the autoconf macro files. Without any arguments it will use the autoconf tools found on your default path. But before you run buildconf, you must...

## Generate-autotools

There's not much to say about installing all these programs. The script `generate-autotools` creates a subfolder called `autotools` within the fpm tree. It performs the next 3 steps for us.

#### Install autoconf version 2.59

There are many versions of autoconf, and we must use _exactly_ the right version, no greater or no less. Php-fpm source tree was generated with autoconf 2.59. To re-generate `configure` script from the macro files therefore requires autoconf 2.59. 

`autoconf --version` will tell you which autoconf you have installed. If your OS is running some other version of autoconf (very likely), then we must install the 2.59 version seperately, into its own directory.

	export AC_VER=`head ./configure | grep -i autoconf | sed -e 's/.*toconf //' -e 's/ .*//' -e 's/\.$//'`
	cd /usr/local/src
	wget http://ftp.gnu.org/gnu/autoconf/autoconf-$AC_VER.tar.gz
	tar -zxvf autoconf-$AC_VER.tar.gz
	cd autoconf-$AC_VER/

Specify the install path with flag --prefix=[DIR] to configure. After all, it would be a bad idea to overwrie the one we already have installed, right? You can use a subdirectory of `/usr/local/src`, your home folder, or perhaps `/opt`. In this example we are going for `/usr/local/src/fpm-autotools`

	mkdir -p /usr/local/src/fpm-autotools
	./configure --prefix=/usr/local/src/fpm-autotools
	make && make install

#### Install automake

	export AM_VER=`head ./Makefile.in | grep -i automake | sed -e 's/.*tomake //' -e 's/ .*//' -e 's/\.$//'`
	wget http://ftp.gnu.org/gnu/automake/automake-$AM_VER.tar.gz
	tar -zxvf automake-$AM_VER.tar.gz
	cd automake-$AM_VER/
	./configure --prefix=/usr/local/src/fpm-autotools
	make && make install
	
#### Install libtool

Libtool is autoconf's partner in crime, and will also be needed.

	export LT_VER=`head -50 ./ltmain.sh | grep "VERSION=" | sed -e 's/^VERSION=//'`
	wget http://ftp.gnu.org/gnu/libtool/libtool-$LT_VER.tar.gz
	tar -zxvf libtool-$LT_VER.tar.gz
	cd libtool-$LT_VER/
	./configure --prefix=/usr/local/src/fpm-autotools
	make && make install

## Edit macro files, acinclude.m4

The first file in our daisy-chain is `acinclude.m4`, which is an m4 marcro file. The layout is divided into sections, such as `AC_DEFUN([AC_FPM_PHP],` and `AC_DEFUN([AC_FPM_LIBEVENT],`. As it depends upon no others it can be safely edited, without risk of breaking the other makefiles and such.

## Regenerate ./configure

After editing a macro file, we can then re-generate `./configure` with the `buildconf` script.

	./buildconf
