DEFS += -DPKGDATADIR=\"$(pkgdatadir)\"

AM_CFLAGS = \
    -I$(top_srcdir)/lib/adt \
    -I$(top_srcdir)/lib/kui \
    -I$(top_srcdir)/lib/rline \
    -I$(top_srcdir)/lib/util \
    -I$(top_srcdir)/lib/tgdb/tgdb-base \
    -I$(top_srcdir)/lib/tgdb/annotate-two \
    -I$(top_srcdir)/lib/tgdb/gdbmi \
    -I$(top_srcdir)/lib/tokenizer

bin_PROGRAMS = cgdb

cgdb_LDFLAGS = \
    -L$(top_builddir)/lib/adt \
    -L$(top_builddir)/lib/kui \
    -L$(top_builddir)/lib/rline \
    -L$(top_builddir)/lib/tokenizer \
    -L$(top_builddir)/lib/util \
    -L$(top_builddir)/lib/tgdb/annotate-two \
    -L$(top_builddir)/lib/tgdb/gdbmi \
    -L$(top_builddir)/lib/tgdb/tgdb-base

cgdb_LDADD = \
    $(top_builddir)/lib/tgdb/tgdb-base/libtgdb.a \
    $(top_builddir)/lib/tgdb/annotate-two/libtgdb_a2.a \
    $(top_builddir)/lib/tgdb/gdbmi/libtgdb_mi.a \
    $(top_builddir)/lib/tokenizer/libtokenizer.a \
    $(top_builddir)/lib/kui/libkui.a \
    $(top_builddir)/lib/rline/librline.a \
    $(top_builddir)/lib/adt/libadt.a \
    $(top_builddir)/lib/util/libutil.a

noinst_HEADERS = \
    command_lexer.h

cgdb_SOURCES = \
    cgdb.c \
    cgdb.h \
    cgdbrc.c \
    cgdbrc.h \
    command_lexer.l \
    filedlg.c \
    filedlg.h \
    highlight.c \
    highlight.h \
    highlight_groups.c \
    highlight_groups.h \
    interface.c \
    interface.h \
    logo.c \
    logo.h \
    scroller.c \
    scroller.h \
    sources.c \
    sources.h \
    usage.c \
    usage.h
