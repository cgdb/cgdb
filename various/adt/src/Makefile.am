AM_CFLAGS = -g -Wall -I$(srcdir)/../../util/include -I$(srcdir)/../include

noinst_LIBRARIES = libadt.a

libadt_a_SOURCES = \
ibuf.c \
queue.c \
tgdb_list.c \
std_hash.c \
std_list.c \
std_tree.c

# Installs the driver programs into progs directory
noinst_bin_PROGRAMS = std_hash_driver std_list_driver std_tree_driver
noinst_bindir = $(top_builddir)/progs

# This is the input driver
std_hash_driver_LDFLAGS = -L.
std_hash_driver_LDADD = -ladt
std_hash_driver_SOURCES = std_hash_driver.c
std_hash_driver_DEPENDENCIES = ../../adt/src
std_hash_driver_CFLAGS = $(AM_CFLAGS)

# This is the input driver
std_list_driver_LDFLAGS = -L.
std_list_driver_LDADD = -ladt
std_list_driver_SOURCES = std_list_driver.c
std_list_driver_DEPENDENCIES = ../../adt/src
std_list_driver_CFLAGS = $(AM_CFLAGS)

# This is the input driver
std_tree_driver_LDFLAGS = -L.
std_tree_driver_LDADD = -ladt
std_tree_driver_SOURCES = std_tree_driver.c
std_tree_driver_DEPENDENCIES = ../../adt/src
std_tree_driver_CFLAGS = $(AM_CFLAGS)
