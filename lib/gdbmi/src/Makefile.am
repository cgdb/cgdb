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
noinst_bin_PROGRAMS = gdbmi_driver
noinst_bindir = $(top_builddir)/progs

gdbmi_driver_LDFLAGS = -L.
gdbmi_driver_LDADD = -lgdbmi
gdbmi_driver_SOURCES = gdbmi_driver.c
gdbmi_driver_DEPENDENCIES = $(top_builddir)/src
