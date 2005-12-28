info_TEXINFOS = cgdb.texinfo
cgdb_TEXINFOS = gpl.texi fdl.texi

SUFFIXES = .html

html_docs = cgdb.html

.texinfo.html:
	$(MAKEINFO) --html $<

.PHONY: html
html: $(html_docs)

clean-local:
	rm -rf *.t2d
