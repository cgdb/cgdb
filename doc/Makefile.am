info_TEXINFOS = cgdb.texinfo
cgdb_TEXINFOS = gpl.texi fdl.texi

dist_man_MANS = cgdb.1
EXTRA_DIST = cgdb.txt htdocs

# Autogenerate the man page using help2man. This happens whenever the user modifies
# either configure.in or usage.c, which contains CGDB's usage.
cgdb.1: $(top_srcdir)/cgdb/src/usage.c $(top_srcdir)/configure.in
	help2man --output=$(top_srcdir)/doc/cgdb.1 $(top_builddir)/cgdb/src/cgdb$(EXEEXT)
