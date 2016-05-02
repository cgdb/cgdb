noinst_LIBRARIES=libgdbmi.a

libgdbmi_a_SOURCES= \
    gdbmi_grammar.ypp \
    gdbmi_lexer.lpp \
    gdbmi_oc.cpp \
    gdbmi_parser.cpp \
    gdbmi_pt.cpp

EXTRA_DIST = \
    gdbmi_grammar.h \
    gdbmi_oc.h \
    gdbmi_parser.h \
    gdbmi_pt.h

# gdbmi driver program
noinst_PROGRAMS = gdbmi_driver

gdbmi_driver_LDADD = libgdbmi.a
gdbmi_driver_SOURCES = gdbmi_driver.cpp
