AM_CFLAGS = -g -Wall -I$(srcdir)/../../../../various/adt/include -I$(srcdir)/../inc

# create the input library
noinst_LIBRARIES = libwm.a
libwm_a_SOURCES = wm.c window.c

# Installs the driver programs into progs directory
noinst_bin_PROGRAMS = wm_driver
noinst_bindir = $(top_builddir)/progs

# This is the wm driver
wm_driver_LDFLAGS = -L. -L../../../../various/adt/src
wm_driver_LDADD = -lwm -ladt
wm_driver_SOURCES = wm_driver.c
wm_driver_CFLAGS = $(AM_CFLAGS)
