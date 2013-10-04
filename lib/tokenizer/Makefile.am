AM_CFLAGS = \
    -I$(top_srcdir)/lib/adt \
    -I$(top_srcdir)/lib/util

#AM_LFLAGS = -Pxx -olex.yy.c

# create the input library
noinst_LIBRARIES = libtokenizer.a
libtokenizer_a_SOURCES = \
	adalexer.l \
	clexer.l \
	dlexer.l \
	golexer.l \
    tokenizer.c \
    tokenizer.h

# Installs the driver programs into progs directory
noinst_PROGRAMS = tokenizer_driver

# This is the input driver
tokenizer_driver_LDFLAGS = -L. \
    -L$(top_builddir)/lib/adt \
    -L$(top_builddir)/lib/util
tokenizer_driver_LDADD = ./libtokenizer.a \
    $(top_builddir)/lib/adt/libadt.a \
    $(top_builddir)/lib/util/libutil.a
tokenizer_driver_SOURCES = tokenizer_driver.c
input_driver_CFLAGS = $(AM_CFLAGS)
