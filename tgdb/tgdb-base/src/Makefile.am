AM_CFLAGS = -g -Wall \
-I$(top_srcdir)/tgdb/tgdb-base/include \
-I$(top_srcdir)/tgdb/annotate-two/include \
-I$(top_srcdir)/tgdb/gdbmi/include \
-I$(top_srcdir)/various/adt/include \
-I$(top_srcdir)/various/util/include \
-I$(top_srcdir)/various/rlctx/include

noinst_LIBRARIES = libtgdb.a

libtgdb_a_SOURCES = \
tgdb.c \
tgdb_types.c \
tgdb_command.c \
tgdb_client_command.c \
tgdb_client_interface.c

noinst_bin_PROGRAMS = tgdb_driver
noinst_bindir = $(top_builddir)/progs

tgdb_driver_LDFLAGS = \
-L$(top_builddir)/tgdb/tgdb-base/src \
-L$(top_builddir)/tgdb/annotate-two/src \
-L$(top_builddir)/tgdb/gdbmi/src \
-L$(top_builddir)/various/adt/src \
-L$(top_builddir)/various/util/src \
-L$(top_builddir)/various/rlctx/src

tgdb_driver_LDADD = -ltgdb -lannotate_two -lgdbmi -lrlctx -ladt -lutil

tgdb_driver_SOURCES = driver.c

tgdb_driver_DEPENDENCIES = \
$(top_srcdir)/various/util/src \
$(top_srcdir)/various/adt/src \
$(top_srcdir)/various/input/src \
$(top_srcdir)/tgdb/annotate-two/src \
$(top_srcdir)/tgdb/gdbmi/src \
$(top_srcdir)/tgdb/tgdb-base/src
