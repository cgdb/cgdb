AM_CFLAGS = -Wall -I$(top_srcdir)/tgdb/include -I$(top_srcdir)/input/include

#EXTRA_PROGRAMS=input_driver getch_driver
#bin_PROGRAMS = @BUILD_INPUT_DRIVER@
bin_PROGRAMS = input_driver getch_driver
input_driver_LDFLAGS = -L ../../tgdb/lib
input_driver_LDADD = -ltgdb
input_driver_SOURCES = input_driver.c input.c
input_driver_DEPENDENCIES = ../../tgdb/lib

# This program provides tgdb with a readline interface
getch_driver_LDFLAGS = -L ../../tgdb/lib
getch_driver_LDADD = -ltgdb
getch_driver_SOURCES = getch_driver.c
getch_driver_DEPENDENCIES = ../../tgdb/lib
