AM_CFLAGS = -Wall
bin_PROGRAMS = cgdb
cgdb_LDFLAGS = -L../tgdb/lib
cgdb_LDADD = -ltgdb
cgdb_SOURCES = cgdb.c highlight.c interface.c filedlg.c \
               scroller.c sources.c logo.c command_lexer.l commands.c

EXTRA_DIST = cgdb.h highlight.h interface.h scroller.h sources.h \
filedlg.h helptext.h logo.h command_lexer.h commands.h

# links cgdb if common is changed
cgdb_DEPENDENCIES = ../tgdb/lib
INCLUDES = -I../tgdb/include
