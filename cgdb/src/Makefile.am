AM_CFLAGS = -g -Wall \
-I$(top_srcdir)/tgdb/tgdb-base/include \
-I$(top_srcdir)/tgdb/annotate-two/include \
-I$(top_srcdir)/tgdb/gdbmi/include \
-I$(top_srcdir)/various/adt/include \
-I$(top_srcdir)/various/util/include \
-I$(top_srcdir)/various/rlctx/include \
-I$(top_srcdir)/various/input/include \
-I$(top_srcdir)/cgdb/include

bin_PROGRAMS = cgdb

cgdb_LDFLAGS = \
-L$(top_srcdir)/various/util/src \
-L$(top_srcdir)/various/adt/src \
-L$(top_srcdir)/various/input/src \
-L$(top_srcdir)/various/rlctx/src \
-L$(top_srcdir)/tgdb/tgdb-base/src \
-L$(top_srcdir)/tgdb/annotate-two/src \
-L$(top_srcdir)/tgdb/gdbmi/src

cgdb_LDADD = -ltgdb -lannotate_two -lgdbmi -lrlctx -linput -ladt -lutil

cgdb_SOURCES = \
cgdb.c \
highlight.c \
interface.c \
filedlg.c \
scroller.c \
sources.c \
logo.c \
command_lexer.l \
commands.c

# links cgdb if common is changed
cgdb_DEPENDENCIES = \
$(top_srcdir)/various/util/src \
$(top_srcdir)/various/adt/src \
$(top_srcdir)/various/input/src \
$(top_srcdir)/various/rlctx/src \
$(top_srcdir)/tgdb/annotate-two/src \
$(top_srcdir)/tgdb/gdbmi/src \
$(top_srcdir)/tgdb/tgdb-base/src
