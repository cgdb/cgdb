AM_CXXFLAGS = -I$(top_srcdir)/lib/util

noinst_LIBRARIES = libadt.a

libadt_a_SOURCES = \
    ibuf.cpp \
    ibuf.h \
    std_bbtree.cpp \
    std_bbtree.h \
    std_list.cpp \
    std_list.h \
    std_types.h

# Installs the driver programs into progs directory
noinst_PROGRAMS = ibuf_driver std_list_driver std_bbtree_driver

# This is the ibuf driver
ibuf_driver_LDFLAGS = -L. -L$(top_builddir)/lib/util
ibuf_driver_LDADD = \
libadt.a \
$(top_builddir)/lib/util/libcgdbutil.a
ibuf_driver_SOURCES = ibuf_driver.cpp
ibuf_driver_CXXFLAGS = $(AM_CXXFLAGS)

# This is the linked list driver
std_list_driver_LDFLAGS = -L. -L$(top_builddir)/lib/util
std_list_driver_LDADD = \
libadt.a \
$(top_builddir)/lib/util/libcgdbutil.a
std_list_driver_SOURCES = std_list_driver.cpp
std_list_driver_CXXFLAGS = $(AM_CXXFLAGS)

# This is the balanced binary tree driver
std_bbtree_driver_LDFLAGS = -L. -L$(top_builddir)/lib/util
std_bbtree_driver_LDADD = \
libadt.a \
$(top_builddir)/lib/util/libcgdbutil.a
std_bbtree_driver_SOURCES = std_bbtree_driver.cpp
std_bbtree_driver_CXXFLAGS = $(AM_CXXFLAGS)
