AM_CFLAGS = -I$(top_srcdir)/tgdb/include -I$(top_srcdir)/tgdb/annotate-two-src -I$(top_srcdir)/tgdb/gdbmi-src

lib_LTLIBRARIES = libtgdb.la

# Bug or feature workaround
tgdb_srcdir = $(top_srcdir)/tgdb

libtgdb_la_SOURCES = \
$(tgdb_srcdir)/tgdb.c \
$(tgdb_srcdir)/tgdb_util.c \
$(tgdb_srcdir)/types.c \
$(tgdb_srcdir)/pseudo.c \
$(tgdb_srcdir)/error.c \
$(tgdb_srcdir)/util.c \
$(tgdb_srcdir)/io.c \
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
$(tgdb_srcdir)/gdbmi-src/gdbmi_init.c \
$(tgdb_srcdir)/gdbmi-src/gdbmi_parse.c

libtgdb_la_LDFLAGS = -avoid-version
