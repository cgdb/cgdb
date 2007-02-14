AM_CFLAGS = -I$(srcdir)/../include \
-I$(top_srcdir)/various/adt/include \
-I$(top_srcdir)/various/util/include

#AM_LFLAGS = -Pxx -olex.yy.c

# create the input library
noinst_LIBRARIES = libtokenizer.a
libtokenizer_a_SOURCES = tokenizer.c clexer.l adalexer.l

# Installs the driver programs into progs directory
noinst_PROGRAMS = tokenizer_driver

# This is the input driver
tokenizer_driver_LDFLAGS = -L. \
-L$(top_builddir)/various/adt/src \
-L$(top_builddir)/various/util/src
tokenizer_driver_LDADD = ./libtokenizer.a \
$(top_builddir)/various/adt/src/libadt.a \
$(top_builddir)/various/util/src/libutil.a
tokenizer_driver_SOURCES = tokenizer_driver.c
input_driver_CFLAGS = $(AM_CFLAGS)
