# The difference between SUBDIRS and DIST_SUBDIRS is that the
# readline directory simply needs to be copied as is, where as
# the rest of the directories have a make dist rule run on them
SUBDIRS=lib various tgdb cgdb doc test
DIST_SUBDIRS=lib various tgdb cgdb doc test

# The order is important here.
# various is the lowest level, so it must be done first.
# TGDB uses various, so it is done next.
# CGDB uses both various and TGDB, so it is done last.
doxygen:
	cd various; doxygen Doxyfile
	cd tgdb; doxygen Doxyfile
	cd cgdb; doxygen Doxyfile

dist-hook:
	rm -rf `find $(distdir) -name .svn`
