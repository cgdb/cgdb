noinst_HEADERS = a2-tgdb.h annotate.h state_machine.h data.h \
terminal.h commands.h globals.h tgdb_init.h buffer.h

EXTRA_PROGRAMS=driver
bin_PROGRAMS = @BUILD_DRIVER@
driver_LDFLAGS = -L.
driver_LDADD = -ltgdb
driver_SOURCES = driver.c
driver_DEPENDENCIES = -ltgdb
INCLUDES = -I../include
