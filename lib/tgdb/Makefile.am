AM_CXXFLAGS = \
    -I$(top_srcdir)/lib/adt \
    -I$(top_srcdir)/lib/rline \
    -I$(top_srcdir)/lib/util \
    -I$(top_srcdir)/lib/tgdb

noinst_LIBRARIES = libtgdb.a

libtgdb_a_SOURCES = \
    annotations.cpp \
    annotations.h \
    tgdb.cpp \
    tgdb.h \
    tgdb_types.h \
    commands.cpp \
    commands.h \
    gdbwire.c \
    gdbwire.h 

noinst_PROGRAMS = tgdb_driver

tgdb_driver_LDFLAGS = \
    -L$(top_builddir)/lib/adt \
    -L$(top_builddir)/lib/rline \
    -L$(top_builddir)/lib/util \
    -L$(top_builddir)/lib/tgdb

tgdb_driver_LDADD = \
    libtgdb.a \
    $(top_builddir)/lib/rline/librline.a \
    $(top_builddir)/lib/adt/libadt.a \
    $(top_builddir)/lib/util/libcgdbutil.a

tgdb_driver_SOURCES = driver.cpp
