AM_CFLAGS = -g -Wall \
-I$(top_srcdir)/various/util/include \
-I$(top_srcdir)/various/adt/include \
-I$(top_srcdir)/tgdb/tgdb-base/include \
-I$(srcdir)/../include

# create the input library
# This depends on input
noinst_LIBRARIES = libgdbmi.a

libgdbmi_a_SOURCES = \
gdbmi_parse.c \
gdbmi_tgdb.c \
gdbmi_grammar.y \
gdbmi_lexer.l \
gdbmi_il.c

# Installs the driver programs into progs directory
noinst_bin_PROGRAMS = gdbmi_driver
noinst_bindir = $(top_builddir)/progs

libgdbmi_a_DEPENDENCIES = \
$(top_srcdir)/various/util/src \
$(top_srcdir)/various/adt/src
 
# This is the input driver
gdbmi_driver_SOURCES = \
	gdbmi_driver.c \
	gdbmi_grammar.y \
	gdbmi_lexer.l \
	gdbmi_il.c

clean:
	rm -f gdbmi_lexer.c \
	gdbmi_grammar.c \
	gdbmi_grammar.h \
	gdbmi_grammar.output \
	gdbmi_grammar.tab.c \
	gdbmi_grammar.tab.h
