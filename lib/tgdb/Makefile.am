AM_CXXFLAGS = \
    -I$(top_srcdir)/lib/util \
    -I$(top_srcdir)/lib/tgdb \
    -std=c++11

noinst_LIBRARIES = libtgdb.a

libtgdb_a_SOURCES = \
    tgdb.cpp \
    tgdb.h \
    gdbwire.c \
    gdbwire.h 

noinst_PROGRAMS = tgdb_driver

tgdb_driver_LDFLAGS = \
    -L$(top_builddir)/lib/util \
    -L$(top_builddir)/lib/tgdb

tgdb_driver_LDADD = \
    libtgdb.a \
    $(top_builddir)/lib/util/libcgdbutil.a

tgdb_driver_SOURCES = driver.cpp
