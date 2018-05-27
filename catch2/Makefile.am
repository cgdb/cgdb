DEFS += -DPKGDATADIR=\"$(pkgdatadir)\" -DTOPBUILDDIR=\"$(abs_top_builddir)\"

AM_CXXFLAGS = \
    -I$(top_srcdir)/cgdb \
    -I$(top_srcdir)/lib/kui \
    -I$(top_srcdir)/lib/rline \
    -I$(top_srcdir)/lib/util \
    -I$(top_srcdir)/lib/tgdb \
    -I$(top_srcdir)/lib/tokenizer

bin_PROGRAMS = tests

tests_LDADD = \
    $(top_builddir)/cgdb/libcgdb.a \
    $(top_builddir)/lib/tgdb/libtgdb.a \
    $(top_builddir)/lib/tokenizer/libtokenizer.a \
    $(top_builddir)/lib/kui/libkui.a \
    $(top_builddir)/lib/rline/librline.a \
    $(top_builddir)/lib/util/libcgdbutil.a

tests_SOURCES = \
    test_main.cpp \
    test_usage.cpp \
    test_logo.cpp \
    test_interface.cpp \
    test_highlight_groups.cpp \
    test_cgdbrc.cpp \
    test_cgdb.cpp \
    test_scroller.cpp \
    test_sources.cpp
