SUBDIRS = various tgdb cgdb
EXTRA_DIST = config

doxygen:
	cd tgdb; doxygen Doxyfile

dist-hook:
	rm -rf `find $(distdir) -name CVS`
