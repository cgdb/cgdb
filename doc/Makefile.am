info_TEXINFOS = cgdb.texi
cgdb_TEXINFOS = gpl.texi

EXTRA_DIST = cgdb.txt

dist_pkgdata_DATA = cgdb.txt

cgdb.txt: cgdb.texi $(srcdir)/version.texi $(cgdb_TEXINFOS)
TEXTS = cgdb.txt
text-am: $(TEXTS)
MAKEINFOTXT = $(MAKEINFO) --plaintext
AM_MAKEINFOTXTFLAGS = $(AM_MAKEINFOFLAGS)

.texi.txt:
	rm -rf $(@:.txt=.htp)
	if $(MAKEINFOTXT) $(AM_MAKEINFOTXTFLAGS) $(MAKEINFOFLAGS) -I $(srcdir) \
	-o $(@:.txt=.htp) $<; \
        then \
          rm -rf $@; \
          if test ! -d $(@:.txt=.htp) && test -d $(@:.txt=); then \
            mv $(@:.txt=) $@; else mv $(@:.txt=.htp) $@; fi; \
        else \
          if test ! -d $(@:.txt=.htp) && test -d $(@:.txt=); then \
            rm -rf $(@:.txt=); else rm -Rf $(@:.txt=.htp) $@; fi; \
          exit 1; \
        fi
