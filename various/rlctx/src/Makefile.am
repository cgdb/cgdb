AM_CFLAGS = -g -Wall \
-I$(top_srcdir)/various/util/include \
-I$(top_srcdir)/various/adt/include \
-I$(srcdir)/../include

# create the input library
noinst_LIBRARIES = librlctx.a
librlctx_a_SOURCES = rlctx.c rlctx_main.c
librlctx_a_DEPENDENCIES = $(top_builddir)/various/util/src $(top_builddir)/various/adt/src

# Installs the driver programs into progs directory
noinst_PROGRAMS = rlctx_driver

# This is the input driver
rlctx_driver_LDFLAGS = -L. -L$(top_builddir)/various/util/src -L$(top_builddir)/various/rlctx/src -L$(top_builddir)/various/adt/src
rlctx_driver_LDADD = -lrlctx -ladt -lutil
rlctx_driver_SOURCES = rlctx_driver.c
rlctx_driver_DEPENDENCIES = $(top_builddir)/various/util/src $(top_builddir)/various/rlctx/src $(top_builddir)/various/adt/src
rlctx_driver_CFLAGS = $(AM_CFLAGS)
