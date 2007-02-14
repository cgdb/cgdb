AM_CFLAGS = -I$(top_srcdir)/various/util/include -I$(top_srcdir)/various/adt/include -I$(srcdir)/../inc

# create the input library
noinst_LIBRARIES = libkui.a
libkui_a_SOURCES = kui.c kui_term.c kui_tree.c

# Installs the driver programs into progs directory
noinst_PROGRAMS = kui_driver

# This is the kui driver
kui_driver_LDFLAGS = -L. -L$(top_builddir)/various/util/src -L$(top_builddir)/various/adt/src
kui_driver_LDADD = libkui.a \
$(top_builddir)/various/util/src/libutil.a \
$(top_builddir)/various/adt/src/libadt.a
kui_driver_SOURCES = kui_driver.c
kui_driver_CFLAGS = $(AM_CFLAGS)
