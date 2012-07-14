noinst_LIBRARIES=libgdbmi.a

libgdbmi_a_SOURCES= \
    gdbmi_grammar.y \
    gdbmi_lexer.l \
    gdbmi_oc.c \
    gdbmi_parser.c \
    gdbmi_pt.c

EXTRA_DIST = \
    gdbmi_grammar.h \
    gdbmi_oc.h \
    gdbmi_parser.h \
    gdbmi_pt.h

# gdbmi driver program
noinst_PROGRAMS = gdbmi_driver

gdbmi_driver_LDADD = libgdbmi.a
gdbmi_driver_SOURCES = gdbmi_driver.c
