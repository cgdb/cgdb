AM_CFLAGS = -I$(top_srcdir)/src/mi_oc_parser

# libgdbmi
noinst_LIBRARIES=libgdbmi.a

libgdbmi_a_SOURCES= \
gdbmi_grammar.y \
gdbmi_lexer.l \
gdbmi_pt.c \
gdbmi_oc.c \
gdbmi_parser.c

EXTRA_DIST = gdbmi_parser.h  gdbmi_pt.h gdbmi_grammar.h gdbmi_oc.h

# gdbmi driver program
noinst_PROGRAMS = gdbmi_driver

gdbmi_driver_LDFLAGS = -L.
gdbmi_driver_LDADD = libgdbmi.a
gdbmi_driver_SOURCES = gdbmi_driver.c
