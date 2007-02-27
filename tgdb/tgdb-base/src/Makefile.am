AM_CFLAGS = \
-I$(top_srcdir)/tgdb/tgdb-base/include \
-I$(top_srcdir)/tgdb/annotate-two/include \
-I$(top_srcdir)/tgdb/gdbmi/include \
-I$(top_srcdir)/various/adt/include \
-I$(top_srcdir)/various/util/include \
-I$(top_srcdir)/various/rline/include

noinst_LIBRARIES = libtgdb.a

libtgdb_a_SOURCES = \
tgdb.c \
tgdb_types.c \
tgdb_command.c \
tgdb_client_interface.c

noinst_PROGRAMS = tgdb_driver

tgdb_driver_LDFLAGS = \
-L$(top_builddir)/tgdb/tgdb-base/src \
-L$(top_builddir)/tgdb/annotate-two/src \
-L$(top_builddir)/tgdb/gdbmi/src \
-L$(top_builddir)/various/adt/src \
-L$(top_builddir)/various/util/src \
-L$(top_builddir)/various/rline/src

tgdb_driver_LDADD = \
libtgdb.a \
$(top_builddir)/tgdb/annotate-two/src/libtgdb_a2.a \
$(top_builddir)/tgdb/gdbmi/src/libtgdb_mi.a \
$(top_builddir)/various/rline/src/librline.a \
$(top_builddir)/various/adt/src/libadt.a \
$(top_builddir)/various/util/src/libutil.a

tgdb_driver_SOURCES = driver.c
