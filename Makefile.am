SUBDIRS = various tgdb cgdb
EXTRA_DIST = config

# The order is important here.
# various is the lowest level, so it must be done first.
# TGDB uses various, so it is done next.
# CGDB uses both various and TGDB, so it is done last.
doxygen:
	cd various; doxygen Doxyfile
	cd tgdb; doxygen Doxyfile
	cd cgdb; doxygen Doxyfile

dist-hook:
	rm -rf `find $(distdir) -name CVS`
