AM_CFLAGS=-Wall
noinst_LIBRARIES = libtgdb.a
libtgdb_a_SOURCES = a2-tgdb.c a2-tgdb.h annotate.c annotate.h \
state_machine.c state_machine.h data.c data.h  \
types.c error.c error.h \
pseudo.c pseudo.h io.c io.h terminal.c terminal.h \
commands.c commands.h globals.c globals.h \
tgdb_init.c tgdb_init.h buffer.c buffer.h \
util.c util.h

EXTRA_PROGRAMS=driver
bin_PROGRAMS = @BUILD_DRIVER@
driver_LDFLAGS = -L.
driver_LDADD = -ltgdb
driver_SOURCES = driver.c
driver_DEPENDENCIES = -ltgdb
INCLUDES = -I../include
