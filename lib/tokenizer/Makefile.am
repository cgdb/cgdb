AM_CXXFLAGS = \
    -I$(top_srcdir)/lib/adt \
    -I$(top_srcdir)/lib/util

#AM_LFLAGS = -Pxx -olex.yy.c

# create the input library
noinst_LIBRARIES = libtokenizer.a
libtokenizer_a_SOURCES = \
    adalexer.lpp \
    clexer.lpp \
    asmlexer.lpp \
    dlexer.lpp \
    golexer.lpp \
    cgdbhelplexer.lpp \
    rustlexer.lpp \
    tokenizer.cpp \
    tokenizer.h

# Installs the driver programs into progs directory
noinst_PROGRAMS = tokenizer_driver

# This is the input driver
tokenizer_driver_LDFLAGS = -L. \
    -L$(top_builddir)/lib/adt \
    -L$(top_builddir)/lib/util
tokenizer_driver_LDADD = ./libtokenizer.a \
    $(top_builddir)/lib/adt/libadt.a \
    $(top_builddir)/lib/util/libcgdbutil.a
tokenizer_driver_SOURCES = tokenizer_driver.cpp
input_driver_CXXFLAGS = $(AM_CXXFLAGS)
