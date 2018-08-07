DEFS += -DPKGDATADIR=\"$(pkgdatadir)\" \
        -DTOPBUILDDIR=\"$(abs_top_builddir)\" \
        -DTESTING

AM_CXXFLAGS = \
    -I$(top_srcdir)/cgdb \
    -I$(top_srcdir)/lib/kui \
    -I$(top_srcdir)/lib/rline \
    -I$(top_srcdir)/lib/util \
    -I$(top_srcdir)/lib/tgdb \
    -I$(top_srcdir)/lib/tokenizer \
    -I$(top_srcdir)/tests/fixtures \
    -I$(top_srcdir)/tests/catch2

bin_PROGRAMS = tests

tests_LDADD = \
    $(top_builddir)/cgdb/libcgdb.a \
    $(top_builddir)/lib/tgdb/libtgdb.a \
    $(top_builddir)/lib/tokenizer/libtokenizer.a \
    $(top_builddir)/lib/kui/libkui.a \
    $(top_builddir)/lib/rline/librline.a \
    $(top_builddir)/lib/util/libcgdbutil.a

tests_SOURCES = \
    tests_main.cpp \
    usage_tests.cpp \
    logo_tests.cpp \
    interface_tests.cpp \
    highlight_groups_tests.cpp \
    cgdbrc_tests.cpp \
    cgdb_tests.cpp \
    scroller_tests.cpp \
    sources_tests.cpp \
    filedlg_tests.cpp \
    highlight_tests.cpp \
    command_lexer_tests.cpp \
    fixtures/curses_fixture.cpp \
    fixtures/logo_fixture.cpp \
    fixtures/cgdbrc_fixture.cpp \
    fixtures/filedlg_fixture.cpp \
    fixtures/sources_fixture.cpp
