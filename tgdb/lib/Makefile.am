AM_CFLAGS = -I$(top_srcdir)/tgdb/include -I$(top_srcdir)/tgdb/annotate-two-src -I$(top_srcdir)/tgdb/gdbmi-src

noinst_LIBRARIES = libtgdb.a

# Bug or feature workaround
tgdb_srcdir = $(top_srcdir)/tgdb

libtgdb_a_SOURCES = \
$(tgdb_srcdir)/lib/tgdb.c \
$(tgdb_srcdir)/lib/tgdb_util.c \
$(tgdb_srcdir)/lib/types.c \
$(tgdb_srcdir)/lib/pseudo.c \
$(tgdb_srcdir)/lib/error.c \
$(tgdb_srcdir)/lib/util.c \
$(tgdb_srcdir)/lib/io.c \
$(tgdb_srcdir)/lib/queue.c \
$(tgdb_srcdir)/annotate-two-src/a2-tgdb.c \
$(tgdb_srcdir)/annotate-two-src/annotate.c \
$(tgdb_srcdir)/annotate-two-src/state_machine.c \
$(tgdb_srcdir)/annotate-two-src/data.c \
$(tgdb_srcdir)/annotate-two-src/terminal.c \
$(tgdb_srcdir)/annotate-two-src/commands.c \
$(tgdb_srcdir)/annotate-two-src/globals.c \
$(tgdb_srcdir)/annotate-two-src/tgdb_init.c \
$(tgdb_srcdir)/annotate-two-src/buffer.c \
$(tgdb_srcdir)/gdbmi-src/gdbmi_tgdb.c \
$(tgdb_srcdir)/gdbmi-src/gdbmi_parse.c

EXTRA_PROGRAMS=driver
bin_PROGRAMS = @BUILD_DRIVER@
driver_LDFLAGS = -L.
driver_LDADD = -ltgdb
driver_SOURCES = driver.c
driver_DEPENDENCIES = -ltgdb
