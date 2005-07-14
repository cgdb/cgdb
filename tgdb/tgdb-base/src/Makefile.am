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

noinst_PROGRAMS = tgdb_driver

tgdb_driver_LDFLAGS = \
-L$(top_builddir)/tgdb/tgdb-base/src \
-L$(top_builddir)/tgdb/annotate-two/src \
-L$(top_builddir)/tgdb/gdbmi/src \
-L$(top_builddir)/various/adt/src \
-L$(top_builddir)/various/util/src \
-L$(top_builddir)/various/rlctx/src

tgdb_driver_LDADD = -ltgdb -lannotate_two -lgdbmi -lrlctx -ladt -lutil
if USE_PACKAGED_READLINE
tgdb_driver_LDADD+=$(top_builddir)/readline-5.0/libreadline.a $(top_builddir)/readline-5.0/libhistory.a
endif

tgdb_driver_SOURCES = driver.c

tgdb_driver_DEPENDENCIES = \
$(top_builddir)/various/util/src \
$(top_builddir)/various/adt/src \
$(top_builddir)/tgdb/annotate-two/src \
$(top_builddir)/tgdb/gdbmi/src \
$(top_builddir)/tgdb/tgdb-base/src
