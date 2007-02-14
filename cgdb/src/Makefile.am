DEFS += -DPKGDATADIR=\"$(pkgdatadir)\"

AM_CFLAGS = \
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

cgdb_LDADD = \
$(top_builddir)/tgdb/tgdb-base/src/libtgdb.a \
$(top_builddir)/tgdb/annotate-two/src/libtgdb_a2.a \
$(top_builddir)/tgdb/gdbmi/src/libtgdb_mi.a \
$(top_builddir)/cgdb/tokenizer/src/libtokenizer.a \
$(top_builddir)/cgdb/lib/kui/src/libkui.a \
$(top_builddir)/various/rline/src/librline.a \
$(top_builddir)/various/adt/src/libadt.a \
$(top_builddir)/various/util/src/libutil.a

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
highlight_groups.c \
usage.c
