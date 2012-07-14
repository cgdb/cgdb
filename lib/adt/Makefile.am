AM_CFLAGS = -I$(top_srcdir)/lib/util

noinst_LIBRARIES = libadt.a

libadt_a_SOURCES = \
    ibuf.c \
    ibuf.h \
    queue.c \
    queue.h \
    tgdb_list.c \
    tgdb_list.h \
    std_bbtree.c \
    std_bbtree.h \
    std_btree.c \
    std_btree.h \
    std_hash.c \
    std_hash.h \
    std_list.c \
    std_list.h \
    std_types.h

# Installs the driver programs into progs directory
noinst_PROGRAMS = ibuf_driver std_hash_driver std_list_driver \
                  std_btree_driver std_bbtree_driver

# This is the ibuf driver
ibuf_driver_LDFLAGS = -L. -L$(top_builddir)/lib/util
ibuf_driver_LDADD = \
libadt.a \
$(top_builddir)/lib/util/libutil.a
ibuf_driver_SOURCES = ibuf_driver.c
ibuf_driver_CFLAGS = $(AM_CFLAGS)

# This is the hash table driver
std_hash_driver_LDFLAGS = -L. -L$(top_builddir)/lib/util
std_hash_driver_LDADD = \
libadt.a \
$(top_builddir)/lib/util/libutil.a
std_hash_driver_SOURCES = std_hash_driver.c
std_hash_driver_CFLAGS = $(AM_CFLAGS)

# This is the linked list driver
std_list_driver_LDFLAGS = -L. -L$(top_builddir)/lib/util
std_list_driver_LDADD = \
libadt.a \
$(top_builddir)/lib/util/libutil.a
std_list_driver_SOURCES = std_list_driver.c
std_list_driver_CFLAGS = $(AM_CFLAGS)

# This is the binary tree driver
std_btree_driver_LDFLAGS = -L. -L$(top_builddir)/lib/util
std_btree_driver_LDADD = \
libadt.a \
$(top_builddir)/lib/util/libutil.a
std_btree_driver_SOURCES = std_btree_driver.c
std_btree_driver_CFLAGS = $(AM_CFLAGS)

# This is the balanced binary tree driver
std_bbtree_driver_LDFLAGS = -L. -L$(top_builddir)/lib/util
std_bbtree_driver_LDADD = \
libadt.a \
$(top_builddir)/lib/util/libutil.a
std_bbtree_driver_SOURCES = std_bbtree_driver.c
std_bbtree_driver_CFLAGS = $(AM_CFLAGS)
