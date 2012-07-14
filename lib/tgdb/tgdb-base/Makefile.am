AM_CFLAGS = \
    -I$(top_srcdir)/lib/adt \
    -I$(top_srcdir)/lib/rline \
    -I$(top_srcdir)/lib/util \
    -I$(top_srcdir)/lib/tgdb/annotate-two \
    -I$(top_srcdir)/lib/tgdb/gdbmi \
    -I$(top_srcdir)/lib/tgdb/tgdb-base

noinst_LIBRARIES = libtgdb.a

libtgdb_a_SOURCES = \
    tgdb.c \
    tgdb.h \
    tgdb_client_interface.c \
    tgdb_client_interface.h \
    tgdb_command.c \
    tgdb_command.h \
    tgdb_types.c \
    tgdb_types.h

noinst_PROGRAMS = tgdb_driver

tgdb_driver_LDFLAGS = \
    -L$(top_builddir)/lib/adt \
    -L$(top_builddir)/lib/rline \
    -L$(top_builddir)/lib/util \
    -L$(top_builddir)/lib/tgdb/annotate-two \
    -L$(top_builddir)/lib/tgdb/gdbmi \
    -L$(top_builddir)/lib/tgdb/tgdb-base

tgdb_driver_LDADD = \
    libtgdb.a \
    $(top_builddir)/lib/tgdb/annotate-two/libtgdb_a2.a \
    $(top_builddir)/lib/tgdb/gdbmi/libtgdb_mi.a \
    $(top_builddir)/lib/rline/librline.a \
    $(top_builddir)/lib/adt/libadt.a \
    $(top_builddir)/lib/util/libutil.a

tgdb_driver_SOURCES = driver.c
