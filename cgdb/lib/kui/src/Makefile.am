AM_CFLAGS = -g -Wall -I$(srcdir)/../../../../various/input/include -I$(srcdir)/../include

# create the input library
# This depends on input
noinst_LIBRARIES = libkui.a
libkui_a_SOURCES = kui.c
libkui_a_DEPENDENCIES = ../../../../various/input/src/libinput.a

# Installs the driver programs into progs directory
noinst_bin_PROGRAMS = kui_driver
noinst_bindir = $(top_srcdir)/progs

# This is the kui driver
kui_driver_LDFLAGS = -L. -L../../../../various/input/src
kui_driver_LDADD = -linput -lkui
kui_driver_SOURCES = kui_driver.c
kui_driver_DEPENDENCIES = ../../../../various/input/src/libinput.a
kui_driver_CFLAGS = $(AM_CFLAGS)
