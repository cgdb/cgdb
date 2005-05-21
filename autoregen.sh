rm -rf autom4te.cache/

aclocal -I config
autoconf -f
autoheader
automake -a
