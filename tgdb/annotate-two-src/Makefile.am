AM_CFLAGS=-Wall
noinst_LIBRARIES = libannotate-two.a
libannotate_two_a_SOURCES = a2-tgdb.c annotate.c state_machine.c data.c \
io.c terminal.c commands.c globals.c tgdb_init.c buffer.c

EXTRA_DIST = a2-tgdb.h annotate.h state_machine.h data.h \
io.h terminal.h commands.h globals.h tgdb_init.h buffer.h

EXTRA_PROGRAMS=driver
bin_PROGRAMS = @BUILD_DRIVER@
driver_LDFLAGS = -L.
driver_LDADD = -lannotate-two
driver_SOURCES = driver.c
driver_DEPENDENCIES = -lannotate-two
INCLUDES = -I../include
