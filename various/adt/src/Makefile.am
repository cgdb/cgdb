AM_CFLAGS = -I$(top_srcdir)/various/util/include -I$(srcdir)/../include

noinst_LIBRARIES = libadt.a

libadt_a_SOURCES = \
ibuf.c \
queue.c \
tgdb_list.c \
std_bbtree.c \
std_btree.c \
std_hash.c \
std_list.c

# Installs the driver programs into progs directory
noinst_PROGRAMS = ibuf_driver std_hash_driver std_list_driver \
                      std_btree_driver std_bbtree_driver

# This is the ibuf driver
ibuf_driver_LDFLAGS = -L.
ibuf_driver_LDADD = -ladt
ibuf_driver_SOURCES = ibuf_driver.c
ibuf_driver_DEPENDENCIES = $(top_builddir)/various/adt/src
ibuf_driver_CFLAGS = $(AM_CFLAGS)

# This is the hash table driver
std_hash_driver_LDFLAGS = -L.
std_hash_driver_LDADD = -ladt
std_hash_driver_SOURCES = std_hash_driver.c
std_hash_driver_DEPENDENCIES = $(top_builddir)/various/adt/src
std_hash_driver_CFLAGS = $(AM_CFLAGS)

# This is the linked list driver
std_list_driver_LDFLAGS = -L.
std_list_driver_LDADD = -ladt
std_list_driver_SOURCES = std_list_driver.c
std_list_driver_DEPENDENCIES = $(top_builddir)/various/adt/src
std_list_driver_CFLAGS = $(AM_CFLAGS)

# This is the binary tree driver
std_btree_driver_LDFLAGS = -L.
std_btree_driver_LDADD = -ladt
std_btree_driver_SOURCES = std_btree_driver.c
std_btree_driver_DEPENDENCIES = $(top_builddir)/various/adt/src
std_btree_driver_CFLAGS = $(AM_CFLAGS)

# This is the balanced binary tree driver
std_bbtree_driver_LDFLAGS = -L.
std_bbtree_driver_LDADD = -ladt
std_bbtree_driver_SOURCES = std_bbtree_driver.c
std_bbtree_driver_DEPENDENCIES = $(top_builddir)/various/adt/src
std_bbtree_driver_CFLAGS = $(AM_CFLAGS)
