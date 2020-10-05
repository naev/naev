ACLOCAL_AMFLAGS = --install -I m4

SUBDIRS = lib src
DIST_SUBDIRS = lib src

if HAVE_UTILS
SUBDIRS += utils
endif # HAVE_NLS
if HAVE_NLS
SUBDIRS += po
DIST_SUBDIRS += po
endif # HAVE_NLS

appicondir = $(datadir)/pixmaps
appicon_DATA = extras/logos/naev.png
Graphicsdir = $(datadir)/applications
Graphics_DATA = naev.desktop
appdatadir = $(datadir)/appdata
appdata_DATA = naev.appdata.xml
dist_pkgdata_DATA = naev-confupdate.sh

dist_man6_MANS = naev.6
dist_doc_DATA = AUTHORS README TODO LICENSE

if HAVE_UTILS
MKSPR      = $(top_srcdir)/utils/mkspr/mkspr$(EXEEXT)
endif
NAEV	     = $(top_srcdir)/$(PACKAGE)$(EXEEXT)

VERSIONFILE = $(top_srcdir)/VERSION
VERSIONDATAFILE = $(top_srcdir)/dat/VERSION
AUTHORSFILE = $(top_srcdir)/dat/AUTHORS
LICENSEFILE = $(top_srcdir)/dat/LICENSE
LANGUAGEFILE = $(top_srcdir)/dat/LANGUAGES

EXTRA_DIST = build/config.rpath LICENSE extras naev.desktop naev.appdata.xml
CLEANFILES = $(NAEV) $(VERSIONFILE) $(VERSIONDATAFILE) $(LANGUAGEFILE) $(LICENSEFILE) $(AUTHORSFILE)

if HAVE_NLS
XMLFILES    = $(top_srcdir)/po/POTFILES_XML.in
TRANSLATIONITS = $(top_srcdir)/po/its/translation.its
XMLPOT      = $(top_srcdir)/po/xml.pot
CLEANFILES += $(XMLPOT)
endif # HAVE_NLS

.PHONY: pot docs utils help install-ndata soundtrack

all-local: $(NAEV) pot $(VERSIONFILE) $(VERSIONDATAFILE) $(LICENSEFILE) $(AUTHORSFILE)

$(AUTHORSFILE): AUTHORS
	@echo "  CP    $<"
	cp $< $@

$(LICENSEFILE): LICENSE
	@echo "  CP    $<"
	cp $< $@

if HAVE_NLS
if HAVE_ITSTOOL
$(XMLPOT): $(XMLFILES) $(TRANSLATIONITS)
	@echo "ITSTOOL $@"
	xargs $(ITSTOOL) -i $(TRANSLATIONITS) < $(XMLFILES) > $@
else # HAVE_ITSTOOL
$(XMLPOT): $(XMLFILES) $(TRANSLATIONITS)
	@echo "XGETTEXT $@"
	$(XGETTEXT) --from-code=UTF-8 --its=$(TRANSLATIONITS) -f $(XMLFILES) -o $@
endif # HAVE_ITSTOOL

if HAVE_DATA
pot: $(XMLPOT) $(LANGUAGEFILE) $(NAEV)
	$(MAKE) -C po $(PACKAGE).pot
	./utils/gettext.sh

$(LANGUAGEFILE): po/LINGUAS
	tail -n +2 $< > $@
else # HAVE_DATA
pot:
endif # HAVE_DATA

else # HAVE_NLS
pot: $(NAEV)
	@echo "***"
	@echo "*** You must have NLS enabled to compile translation files"
	@echo "***"
endif # HAVE_NLS

$(NAEV):
	@echo "  LN    $(NAEV)"
	$(LN_S) "src/$(NAEV)" "$(NAEV)"

$(VERSIONFILE): src/config.h
	@echo $(VERSION) > $@

$(VERSIONDATAFILE): src/config.h
	@echo $(VERSION) > $@

if HAVE_UTILS
utils: $(MKSPR)

$(MKSPR):
	$(MAKE) -C utils/mkspr
else # HAVE_UTILS
utils:
endif # HAVE_UTILS

install-data-local:
	(cd $(top_srcdir)/dat; find . -type f -exec install -Dm 644 "{}" "$(pkgdatadir)/dat/{}" \;)

uninstall-hook:
	echo " rm -rf '$(DESTDIR)$(pkgdatadir)'"; \
	echo " rm -rf '$(docdir)'"; \
	rm -rf "$(DESTDIR)$(pkgdatadir)"
	rm -rf "$(docdir)"

docs:
	$(MAKE) -C docs

soundtrack: naev-$(VERSION)-soundtrack.zip

naev-$(VERSION)-soundtrack.zip: utils/soundtrack.py dat/snd/soundtrack.yaml
	./utils/soundtrack.py

help:
	@echo "Possible targets are:"
	@echo "        all - builds everything"
	@echo "       docs - creates the doxygen documentation"
	@echo " soundtrack - creates the soundtrack zip file"
	@echo "      clean - removes binaries and object files"
	@echo "    install - installs naev"
	@echo "  uninstall - removes previously installed files"
	@echo "  dist-gzip - creates a source tarball with gzip compression"
	@echo " dist-bzip2 - creates a source tarball with bzip2 compression"
	@echo "  distcheck - checks if the created tarball works properly"
	@echo "  distclean - does clean and removes files created by configure"
