AM_CFLAGS = -g -Wall -I../../util/include -I../include

# create the input library
# This depends on input
noinst_LIBRARIES = libinput.a
libinput_a_SOURCES = input.c
libinput_a_DEPENDENCIES = ../../util/src

EXTRA_PROGRAMS=input_driver getch_driver
bin_PROGRAMS = @BUILD_INPUT_DRIVERS@

# This is the input driver
input_driver_LDFLAGS = -L. -L../../util/src
input_driver_LDADD = -linput -lutil
input_driver_SOURCES = input_driver.c
input_driver_DEPENDENCIES = ../../util/src libinput.a
input_driver_CFLAGS = $(AM_CFLAGS)

# This program provides tgdb with a readline interface
getch_driver_LDFLAGS = -L ../../util/src
getch_driver_LDADD = -lutil
getch_driver_SOURCES = getch_driver.c
getch_driver_DEPENDENCIES = ../../util/src
getch_driver_CFLAGS = $(AM_CFLAGS)
