AM_CFLAGS = -I ../tgdb/annotate-two-src -Wall
bin_PROGRAMS = cgdb
cgdb_LDFLAGS = -L../tgdb/annotate-two-src -L../tgdb/src
cgdb_LDADD = -ltgdb -lannotate-two -ltgdb
cgdb_SOURCES = cgdb.c highlight.c interface.c filedlg.c \
               scroller.c sources.c logo.c command_lexer.l commands.c

EXTRA_DIST = cgdb.h highlight.h interface.h scroller.h sources.h \
filedlg.h helptext.h logo.h command_lexer.h commands.h

# links cgdb if common is changed
cgdb_DEPENDENCIES = ../tgdb/annotate-two-src ../tgdb/src
INCLUDES = -I../tgdb/include
