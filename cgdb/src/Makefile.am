AM_CFLAGS = -g -Wall \
-I$(top_srcdir)/tgdb/tgdb-base/include \
-I$(top_srcdir)/tgdb/annotate-two/include \
-I$(top_srcdir)/tgdb/gdbmi/include \
-I$(top_srcdir)/various/adt/include \
-I$(top_srcdir)/various/util/include \
-I$(top_srcdir)/various/rline/include \
-I$(top_srcdir)/cgdb/lib/kui/inc \
-I$(top_srcdir)/cgdb/tokenizer/include \
-I$(top_srcdir)/cgdb/include

bin_PROGRAMS = cgdb

cgdb_LDFLAGS = \
-L$(top_builddir)/various/util/src \
-L$(top_builddir)/various/adt/src \
-L$(top_builddir)/various/rline/src \
-L$(top_builddir)/tgdb/tgdb-base/src \
-L$(top_builddir)/tgdb/annotate-two/src \
-L$(top_builddir)/cgdb/lib/kui/src \
-L$(top_builddir)/cgdb/tokenizer/src \
-L$(top_builddir)/tgdb/gdbmi/src

cgdb_LDADD = -ltgdb -lannotate_two -lgdbmi -ltokenizer -lkui -lrline -ladt -lutil

cgdb_SOURCES = \
cgdb.c \
highlight.c \
interface.c \
filedlg.c \
scroller.c \
sources.c \
logo.c \
command_lexer.l \
cgdbrc.c \
usage.c

# links cgdb if common is changed
cgdb_DEPENDENCIES = \
$(top_srcdir)/various/util/src \
$(top_srcdir)/various/adt/src \
$(top_srcdir)/various/rline/src \
$(top_srcdir)/cgdb/lib/kui/src \
$(top_srcdir)/cgdb/tokenizer/src \
$(top_srcdir)/tgdb/annotate-two/src \
$(top_srcdir)/tgdb/gdbmi/src \
$(top_srcdir)/tgdb/tgdb-base/src
