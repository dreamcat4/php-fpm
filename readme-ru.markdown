# PHP FastCGI Менеджер процессов (PHP-FPM)

PHP-FPM это Fast-CGI фронтэнд для php и расширение php-cgi. Проект находится на [Launchpad](https://launchpad.net/php-fpm)

## Быстрый старт:

Выберите один из 2 путей сборки fpm: Или `встроенный`, или `отдельный`. Если вы не разработчик или не системный администратор, то мы рекомендуем `встроенный` вариант компиляции. Для дополнительной информации смотрите файл `readme.markdown`.

## Зависимости
Если вы до этого не устанавливали php, то вам придётся установить пакет `libxml2-dev`. FPM также необходим `libevent-dev`. Debian / ubuntu:

	sudo aptitude install -y libxml2-dev libevent-dev

Рекомендуется использовать libevent 1.4.12-stable или позднее, но необходим, как минимум, libevent 1.4.3-stable. Если нет подходящей версии, скайте и скомпилируйте с [сайта Libevent](http://www.monkey.org/~provos/libevent/).

	export LE_VER=1.4.12-stable
	wget "http://www.monkey.org/~provos/libevent-$LE_VER.tar.gz"
	tar -zxvf "libevent-$LE_VER.tar.gz"
	cd "libevent-$LE_VER"
	./configure && make
	DESTDIR=$PWD make install
	export LIBEVENT_SEARCH_PATH="$PWD/usr/local"

## Встроенная сборка

Скачайте fpm и сгенерируйте патч

	export PHP_VER=5.3.0
	wget "http://launchpad.net/php-fpm/master/0.6/+download/php-fpm-0.6~$PHP_VER.tar.gz"
	tar -zxvf "php-fpm-0.6~$PHP_VER.tar.gz"
	"php-fpm-0.6~$PHP_VER/generate-fpm-patch"

Скачайте и распакуйте исходный код PHP

	wget "http://ru2.php.net/get/php-$PHP_VER.tar.gz/from/ru2.php.net/mirror"
	tar xvfz "php-$PHP_VER.tar.gz"
	cd "php-$PHP_VER"

Примените патч и компилируйте

	patch -p1 < ../fpm.patch
	./buildconf --force
	mkdir fpm-build && cd fpm-build
	../configure --with-fpm \
	--with-libevent="$LIBEVENT_SEARCH_PATH" && make

## Отдельная сборка

Скачайте и распакуйте исходный код PHP

	export PHP_VER=5.3.0
	wget "http://ru2.php.net/get/php-$PHP_VER.tar.gz/from/ru2.php.net/mirror"
	tar xvfz "php-$PHP_VER.tar.gz"
	cd "php-$PHP_VER"
	mkdir php-build && cd php-build
	../configure && make

Теперь можете скачать, конфигурировать и компилировать FPM фронтэнд

	wget "http://launchpad.net/php-fpm/master/0.6/+download/php-fpm-0.6~$PHP_VER.tar.gz"
	tar -zxvf "php-fpm-0.6~$PHP_VER.tar.gz"
	cd "php-fpm-0.6~$PHP_VER"
	mkdir fpm-build && cd fpm-build
	../configure --srcdir=../ \
	 --with-php-src="../../php-$PHP_VER" \
	 --with-php-build="../../php-$PHP_VER/php-build" \
	 --with-libevent="$LIBEVENT_SEARCH_PATH" && make

## Php Configure flags

	--with-fpm                   			Build the fpm SAPI (and not php-cgi)
	--with-config-file-path=[PATH]			Where to look for php.ini
	--with-config-file-scan-dir[=PATH]		Search path for extension .ini files

There are many possible php build flags. Please consult the official php documentation.

* Note:
  The following build flags are not used anymore. They are simply ignored by the configure script.

		--enable-fastcgi
		--enable-force-cgi-redirect

## FPM Флаги конфигурирования

	--with-libevent[=PATH]       Путь до libevent, для fpm SAPI [/usr/local]
	--with-fpm-bin[=PATH]        Путь для откомпилированного php-fpm [/usr/local/bin/php-fpm]
	--with-fpm-port[=PORT]       TCP порт для cgi запросов [9000]
	--with[out]-fpm-conf[=PATH]  Путь до файла конфигурации php-fpm [/etc/php-fpm.conf]
	--with[out]-fpm-init[=PATH]  Путь до init-файла php-fpm [/etc/init.d/php-fpm]
	--with-fpm-log[=PATH]        Путь до лог-файла php-fpm [/var/log/php-fpm.log]
	--with-fpm-pid[=PATH]        Путь до pid-файла php-fpm [/var/run/php-fpm.pid]
	--with-fpm-user[=USER]       Пользователь, под которым запускать php-fpm [nobody]
	--with-fpm-group[=GRP]       Группа, под которой запускать php-fpm. Для системных 
		                  	     пользователей задайте имя пользователя [nobody]

## Установка

Если вы делали `встроенную` сборку, то вы получите полный php, включая исполнитель коммандной строки `php-cli` и библиотеку PEAR. `Отдельная` или `независимая` сборка установит только демон `php-fpm` и минимум файлов, необходимых для его запуска.

	# Посмотреть, какие файлы будут установлены
	make install --dry-run

	# Установить в '/'
	sudo make install

	# Установить в '/opt'
	sudo INSTALL_ROOT=/opt make install

Notes:

* (Upgrade) When overwriting existing FPM installation files: A previous configuration file `php-fpm.conf` will be moved to `php-fpm.conf.old`. Then a newer (default) configuration file will be installed in it's place. If you have any custom XML settings which you wish to keep, its recommended to copy these back over manually.

* (BSD) the default init.d path is `/usr/local/etc/rc.d/php-fpm` or disable: `--without-fpm-init`

* (Nginx) An example nginx configuration file is generated. The file `nginx-site-conf.sample` may be installed into your nginx configuration directory, if exists: `/etc/nginx/`, `/usr/local/etc/nginx/`, or `/usr/local/nginx/conf`

## Больше о процессе сборки PHP-FPM

Процесс сборки можно описать так:

	 1) Компилируются исходники php в объектные файлы
	 2) Компилируются исходники fpm в объектные файлы
	 3) Линковка php и fpm объектных файлов
	 4) Результат: исполняемый php5, в основе которого php и fast-CGI от fpm как фронтэнд

Fpm подмешивается в php при линковке (link-level). Андрей разделил исходный код fpm, сделав SAPI чем-то менее чуствительным к изменениям в остальном коде php. Код cgi-main.c из PHP-FPM - конктроллер запросов - вырезан из оригинального fcgi-sapi. Мы отправляем билд 0.6 в PHP Group. Мы будем отслеживать развитие PHP и периодически синхронизировать изменения с проектами встроенной / отдельной сборки.

## Buildconf

Для сборки fpm отдельно, конфигурирование (`./configure`) требует некоторой версии набора инструментов autoconf. Buildconf запустит `./generate-autotools` и попробует установить эти инструменты самостоятельно. Если `./buildconf` не работает, смотрите лог ошибок.


## Обсуждение

Есть 2 группы для обсуждения php-fpm,

- [highload-php-ru](http://groups.google.com/group/highload-php-en) (english)

- [highload-php-ru](http://groups.google.com/group/highload-php-ru) (русская)

Translated by Anatoly Pashin
