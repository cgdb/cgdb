AM_CFLAGS=-Wall
noinst_LIBRARIES = libtgdb.a
libtgdb_a_SOURCES = tgdb.c tgdb.h annotate.c annotate.h \
state_machine.c state_machine.h data.c data.h  \
types.c types.h error.c error.h \
pseudo.c pseudo.h io.c io.h terminal.c terminal.h \
commands.c commands.h globals.c globals.h \
macro.h macro.c tgdb_init.c tgdb_init.h buffer.c buffer.h \
util.c util.h

bin_PROGRAMS = driver
driver_LDFLAGS = -L.
driver_LDADD = -ltgdb
driver_SOURCES = driver.c
driver_DEPENDENCIES = -ltgdb
INCLUDES = -I../include
