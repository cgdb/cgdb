AM_CFLAGS = -I ../tgdb -Wall
bin_PROGRAMS = cgdb
cgdb_LDFLAGS = -L../tgdb
cgdb_LDADD = -ltgdb
cgdb_SOURCES = cgdb.c cgdb.h highlight.c highlight.h interface.c filedlg.c \
               interface.h scroller.c scroller.h sources.c sources.h \
               filedlg.h helptext.h logo.c logo.h

# links cgdb if common is changed
cgdb_DEPENDENCIES = ../tgdb
