AM_CFLAGS = -g -Wall \
-I$(top_srcdir)/various/util/include \
-I$(top_srcdir)/various/adt/include \
-I$(top_srcdir)/various/rlctx/include \
-I$(top_srcdir)/tgdb/tgdb-base/include \
-I../include

# create the input library
# This depends on input
noinst_LIBRARIES = libannotate_two.a

libannotate_two_a_SOURCES = \
a2-tgdb.c \
annotate.c \
buffer.c \
commands.c \
data.c \
globals.c \
state_machine.c \
terminal.c \
tgdb_init.c

libannotate_two_a_DEPENDENCIES = \
$(top_srcdir)/various/util/src \
$(top_srcdir)/various/adt/src \
$(top_srcdir)/various/rlctx/src
