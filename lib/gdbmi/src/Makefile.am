AM_CFLAGS = -I$(top_srcdir)/src/mi_oc_parser

# libgdbmi
noinst_LIBRARIES=libgdbmi.a

libgdbmi_a_SOURCES= \
gdbmi_grammar.y \
gdbmi_lexer.l \
gdbmi_pt.c \
gdbmi_parser.c

# gdbmi driver program
noinst_bin_PROGRAMS = gdbmi_driver
noinst_bindir = $(top_builddir)/progs

gdbmi_driver_LDFLAGS = -L.
gdbmi_driver_LDADD = -lgdbmi
gdbmi_driver_SOURCES = gdbmi_driver.c
#gdbmi_driver_DEPENDENCIES = $(top_srcdir)/src/mi_oc_parser
#

#mi_oc_parser_LDFLAGS=-L.
#mi_oc_parser_LDADD=-lmi_oc_parser
#mi_oc_parser_SOURCES=mi_oc_driver.c
#mi_oc_parser_CFLAGS=$(AM_CLFAGS)
