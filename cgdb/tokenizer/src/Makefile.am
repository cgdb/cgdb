AM_CFLAGS = -g -Wall -I$(srcdir)/../include \
-I$(top_srcdir)/various/adt/include \
-I$(top_srcdir)/various/util/include

#AM_LFLAGS = -Pxx -olex.yy.c

# create the input library
noinst_LIBRARIES = libtokenizer.a
libtokenizer_a_SOURCES = tokenizer.c clexer.l adalexer.l
libtokenizer_a_DEPENDENCIES= \
$(top_srcdir)/various/adt/src \
$(top_srcdir)/various/util/src

# Installs the driver programs into progs directory
noinst_bin_PROGRAMS = tokenizer_driver
noinst_bindir = $(top_srcdir)/progs

# This is the input driver
tokenizer_driver_LDFLAGS = -L. \
-L$(top_builddir)/various/adt/src \
-L$(top_builddir)/various/util/src
tokenizer_driver_LDADD = -ltokenizer -ladt -lutil
tokenizer_driver_SOURCES = tokenizer_driver.c
tokenizer_driver_DEPENDENCIES = libtokenizer.a
input_driver_CFLAGS = $(AM_CFLAGS)
