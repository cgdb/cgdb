AM_CFLAGS = -g -Wall \
-I$(top_srcdir)/various/util/include \
-I$(top_srcdir)/various/adt/include \
-I../include

# create the input library
noinst_LIBRARIES = librlctx.a
librlctx_a_SOURCES = rlctx.c rlctx_main.c
librlctx_a_DEPENDENCIES = ../../util/src ../../adt/src
