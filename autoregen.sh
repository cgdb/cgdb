rm -rf autom4te.cache/

aclocal -I .
autoconf
autoheader
automake -a
