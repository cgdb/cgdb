info_TEXINFOS = cgdb.texinfo
cgdb_TEXINFOS = gpl.texi fdl.texi

dist_man_MANS = cgdb.1
EXTRA_DIST = cgdb.txt htdocs

# generate the man page using help2man.
cgdb.1:
	help2man --output=$(top_srcdir)/doc/cgdb.1 $(top_builddir)/cgdb/src/cgdb$(EXEEXT)
