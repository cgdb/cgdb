AM_CFLAGS = \
	-I$(top_srcdir)/lib/adt \
    -I$(top_srcdir)/lib/util

# create the input library
noinst_LIBRARIES = libkui.a
libkui_a_SOURCES = \
    kui.c \
    kui.h \
    kui_cgdb_key.h \
	kui_term.c \
	kui_term.h \
	kui_tree.c \
	kui_tree.h

# Installs the driver programs into progs directory
noinst_PROGRAMS = kui_driver

# This is the kui driver
kui_driver_LDFLAGS = \
	-L$(top_builddir)/lib/adt \
    -L$(top_builddir)/lib/util
kui_driver_LDADD = libkui.a \
    $(top_builddir)/lib/adt/libadt.a \
    $(top_builddir)/lib/util/libutil.a
kui_driver_SOURCES = kui_driver.c
kui_driver_CFLAGS = $(AM_CFLAGS)
