AM_CFLAGS = -g -Wall \
-I$(top_srcdir)/various/util/include \
-I$(top_srcdir)/various/adt/include \
-I$(top_srcdir)/tgdb/tgdb-base/include \
-I$(srcdir)/../include

# create the input library
# This depends on input
noinst_LIBRARIES = libgdbmi.a

libgdbmi_a_SOURCES = gdbmi_tgdb.c

libgdbmi_a_DEPENDENCIES = \
$(top_srcdir)/various/util/src \
$(top_srcdir)/various/adt/src
