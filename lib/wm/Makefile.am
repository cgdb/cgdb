AM_CFLAGS = -I$(top_srcdir)/lib/adt

# create the input library
noinst_LIBRARIES = libwm.a
libwm_a_SOURCES = \
    window.c \
    window.h \
    wm.c \
    wm.h

# Installs the driver programs into progs directory
noinst_PROGRAMS = wm_driver

# This is the wm driver
wm_driver_LDFLAGS = -L. -L$(top_builddir)/lib/adt
wm_driver_LDADD = -lwm -ladt
wm_driver_SOURCES = wm_driver.c
wm_driver_CFLAGS = $(AM_CFLAGS)
