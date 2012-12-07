AM_CFLAGS = -I$(top_srcdir)/lib/adt

# create the input library
noinst_LIBRARIES = libwm.a
libwm_a_SOURCES = \
    wm.c \
    wm.h \
    wm_splitter.c \
    wm_splitter.h \
    wm_window.c \
    wm_window.h \
    types.h

# Installs the driver programs into progs directory
noinst_PROGRAMS = wm_driver

# This is the wm driver
wm_driver_LDFLAGS =
wm_driver_LDADD = libwm.a
wm_driver_SOURCES = wm_driver.c
wm_driver_CFLAGS = $(AM_CFLAGS)
