AM_CFLAGS = -I ../tgdb/annotate-two-src -I ../tgdb/gdb-mi-src -Wall
bin_PROGRAMS = cgdb
cgdb_LDFLAGS = -L../tgdb/annotate-two-src -L../tgdb/gdb-mi-src
cgdb_LDADD = -ltgdb
cgdb_SOURCES = cgdb.c cgdb.h highlight.c highlight.h interface.c filedlg.c \
               interface.h scroller.c scroller.h sources.c sources.h \
               filedlg.h helptext.h logo.c logo.h command_lexer.l commands.c

# links cgdb if common is changed
cgdb_DEPENDENCIES = ../tgdb/annotate-two-src ../tgdb/gdb-mi-src
INCLUDES = -I../tgdb/include
