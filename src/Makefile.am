AM_CFLAGS = -I ../tgdb/annotate-two-src -Wall
bin_PROGRAMS = cgdb
cgdb_LDFLAGS = -L../tgdb/annotate-two-src
cgdb_LDADD = -ltgdb
cgdb_SOURCES = cgdb.c cgdb.h highlight.c highlight.h interface.c filedlg.c \
               interface.h scroller.c scroller.h sources.c sources.h \
               filedlg.h helptext.h logo.c logo.h \
               command_lexer.l configure_lexer.h commands.c commands.h

# links cgdb if common is changed
cgdb_DEPENDENCIES = ../tgdb/annotate-two-src
INCLUDES = -I../tgdb/include
