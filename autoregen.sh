rm -rf autom4te.cache/

aclocal
autoconf
autoheader
automake -a

#touch Makefile.in
#touch src/Makefile.in
#touch tgdb/Makefile.in
#touch tgdb/src/Makefile.in

#touch config.status
#touch stamp-h.in
#touch config-h.in
#touch configure
#touch aclocal.m4

#touch Makefile.am
#touch src/Makefile.am
#touch tgdb/Makefile.am
#touch tgdb/src/Makefile.am

#touch configure.in

#cvs commit -f Makefile.in src/Makefile.in tgdb/Makefile.in tgdb/src/Makefile.in \
#              config.status stamp-h.in config-h.in configure aclocal.m4 \
#              Makefile.am src/Makefile.am tgdb/Makefile.am tgdb/src/Makefile.am \
#              configure.in
