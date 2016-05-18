AM_CXXFLAGS = \
    -I$(top_srcdir)/lib/adt \
    -I$(top_srcdir)/lib/util

# create the input library
noinst_LIBRARIES = libkui.a
libkui_a_SOURCES = \
    kui.cpp \
    kui.h \
    kui_cgdb_key.h \
    kui_term.cpp \
    kui_term.h \
    kui_tree.cpp \
    kui_tree.h

# Installs the driver programs into progs directory
noinst_PROGRAMS = kui_driver

# This is the kui driver
kui_driver_LDFLAGS = \
    -L$(top_builddir)/lib/adt \
    -L$(top_builddir)/lib/util
kui_driver_LDADD = libkui.a \
    $(top_builddir)/lib/adt/libadt.a \
    $(top_builddir)/lib/util/libcgdbutil.a
kui_driver_SOURCES = kui_driver.cpp
kui_driver_CXXFLAGS = $(AM_CXXFLAGS)
