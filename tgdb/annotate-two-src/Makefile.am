AM_CFLAGS=-Wall
noinst_LIBRARIES = libtgdb.a
libtgdb_a_SOURCES = a2-tgdb.c annotate.c \
state_machine.c data.c types.c error.c \
pseudo.c io.c terminal.c commands.c globals.c \
tgdb_init.c buffer.c util.c

EXTRA_DIST = a2-tgdb.h annotate.h \
state_machine.h data.h error.h \
pseudo.h io.h terminal.h commands.h globals.h \
tgdb_init.h buffer.h util.h

EXTRA_PROGRAMS=driver
bin_PROGRAMS = @BUILD_DRIVER@
driver_LDFLAGS = -L.
driver_LDADD = -ltgdb
driver_SOURCES = driver.c
driver_DEPENDENCIES = -ltgdb
INCLUDES = -I../include
