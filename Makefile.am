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
nobase_dist_pkgdata_DATA = dat/* po/*

if HAVE_UTILS
MKSPR      = utils/mkspr/mkspr$(EXEEXT)
endif
NAEV	     = $(PACKAGE)$(EXEEXT)

VERSIONFILE = VERSION
VERSIONDATAFILE = dat/VERSION
LANGUAGEFILE = $(top_srcdir)/dat/LANGUAGES

EXTRA_DIST = build/config.rpath LICENSE extras naev.desktop naev.appdata.xml
CLEANFILES = $(NAEV) $(VERSIONFILE) $(VERSIONDATAFILE) $(LANGUAGEFILE)

if HAVE_NLS
XMLBASE     = $(wildcard dat/*.xml)
XMLOUTFITS  = $(wildcard dat/outfits/*/*.xml)
XMLSHIPS    = $(wildcard dat/ships/*.xml)
XMLFILES    = $(XMLBASE) $(XMLSHIPS) $(XMLOUTFITS)
TRANSLATIONITS = $(top_srcdir)/po/translation.its
XMLPOT      = $(top_srcdir)/po/xml.pot
CLEANFILES += $(XMLPOT)
endif # HAVE_NLS

.PHONY: pot docs utils help install-ndata soundtrack

all-local: $(NAEV) pot $(VERSIONFILE) $(VERSIONDATAFILE)

if HAVE_NLS
$(XMLPOT): $(XMLFILES) $(TRANSLATIONITS)
	$(XGETTEXT) --from-code=UTF-8 --its=$(TRANSLATIONITS) $(XMLFILES) -o $@
	@echo "XGETTEXT $@"

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
	echo $(VERSION) > $@

$(VERSIONDATAFILE): src/config.h
	echo $(VERSION) > $@

if HAVE_UTILS
utils: $(MKSPR)

$(MKSPR):
	$(MAKE) -C utils/mkspr
else # HAVE_UTILS
utils:
endif # HAVE_UTILS

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
