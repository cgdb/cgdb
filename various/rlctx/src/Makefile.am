AM_CFLAGS = -g -Wall \
-I$(top_srcdir)/various/util/include \
-I$(top_srcdir)/various/adt/include \
-I../include

#EXTRA_PROGRAMS=input_driver getch_driver
#bin_PROGRAMS = @BUILD_INPUT_DRIVER@

# create the input library
# This depends on input
noinst_LIBRARIES = librlctx.a
librlctx_a_SOURCES = rlctx.c
librlctx_a_DEPENDENCIES = ../../util/src ../../adt/src

bin_PROGRAMS = rlctx_prog

# This is the input driver
rlctx_prog_LDFLAGS = -L. -L../../util/src -L../../adt/src
rlctx_prog_LDADD = -lrlctx -ladt -lutil
rlctx_prog_SOURCES = rlctx_prog.c
rlctx_prog_DEPENDENCIES = ../../util/src ../../adt/src librlctx.a

# This program provides tgdb with a readline interface
# There should be a driver program to test the functionality of rlctx
#getch_driver_LDFLAGS = -L ../../util/src
#getch_driver_LDADD = -lutil
#getch_driver_SOURCES = getch_driver.c
#getch_driver_DEPENDENCIES = ../../util/src
