rm -rf autom4te.cache/

aclocal -I config
autoconf
autoheader
automake -a
