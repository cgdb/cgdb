AM_CFLAGS = -g -Wall \
-I$(top_srcdir)/various/util/include \
-I$(top_srcdir)/various/adt/include \
-I../include

# create the input library
noinst_LIBRARIES = librlctx.a
librlctx_a_SOURCES = rlctx.c rlctx_main.c
librlctx_a_DEPENDENCIES = ../../util/src ../../adt/src

# Installs the driver programs into progs directory
noinst_bin_PROGRAMS = rlctx_driver
noinst_bindir = $(top_srcdir)/progs

# This is the input driver
rlctx_driver_LDFLAGS = -L. -L../../util/src -L../../rlctx/src -L../../adt/src
rlctx_driver_LDADD = -lrlctx -ladt -lutil
rlctx_driver_SOURCES = rlctx_driver.c
rlctx_driver_DEPENDENCIES = ../../util/src ../../rlctx/src ../../adt/src
rlctx_driver_CFLAGS = $(AM_CFLAGS)
