ACLOCAL_AMFLAGS = --install -I m4

SUBDIRS = lib src
DIST_SUBDIRS = lib src utils

if HAVE_NLS
SUBDIRS += po
DIST_SUBDIRS += po
endif # HAVE_NLS

appicondir = $(datadir)/icons/hicolor/256x256/apps
appicon_DATA = extras/logos/naev.png
Graphicsdir = $(datadir)/applications
Graphics_DATA = org.naev.naev.desktop
appdatadir = $(datadir)/metainfo
appdata_DATA = org.naev.naev.metainfo.xml
dist_pkgdata_DATA = naev-confupdate.sh

dist_man6_MANS = naev.6
dist_doc_DATA = README TODO LICENSE

MKSPR      = $(top_builddir)/utils/mkspr/mkspr$(EXEEXT)
NAEV	     = $(top_builddir)/$(PACKAGE)$(EXEEXT)

VERSIONFILE = $(top_builddir)/dat/VERSION
LICENSEFILE = $(top_builddir)/dat/LICENSE
LANGUAGEFILE = $(top_builddir)/dat/LANGUAGES

EXTRA_DIST = build/config.rpath LICENSE extras dat utils org.naev.naev.desktop org.naev.naev.metainfo.xml
CLEANFILES = $(NAEV) $(VERSIONFILE) $(LANGUAGEFILE) $(LICENSEFILE)

if HAVE_NLS
XMLFILES    = $(top_srcdir)/po/POTFILES_XML.in
TRANSLATIONITS = $(top_srcdir)/po/its/translation.its
CREDITSPOT  = $(top_builddir)/po/credits.pot
CREDITSTEXT = $(top_srcdir)/dat/AUTHORS $(top_srcdir)/dat/intro
NAEVSRCPOT  = $(top_builddir)/po/$(GETTEXT_PACKAGE).pot
NAEVPOT     = $(top_builddir)/po/$(PACKAGE).pot
XMLPOT      = $(top_builddir)/po/xml.pot
EXTRA_DIST += $(XMLFILES) $(TRANSLATIONITS)
CLEANFILES += $(CREDITSPOT) $(NAEVPOT) $(XMLPOT)
endif # HAVE_NLS

.PHONY: data pot docs utils help install-ndata soundtrack

all-local: $(NAEV) data

data: pot $(VERSIONFILE) $(LICENSEFILE)

$(LICENSEFILE): $(top_srcdir)/LICENSE
	@echo "  CP    $<"
	@mkdir -p $(top_builddir)/dat
	cp $< $@

if HAVE_NLS
$(CREDITSPOT): $(CREDITSTEXT)
	@echo "  AWK   $@"
	awk -f $(top_srcdir)/utils/update-po-credits.awk $(CREDITSTEXT) > $@
if HAVE_ITSTOOL
# On windows it seems like '\r' are leaking into filenames and breaking the
# following command, so we use lots of ugly tr instead ¯\_(ツ)_/¯
# xargs -a $(XMLFILES) $(ITSTOOL) -i $(TRANSLATIONITS) > $@
$(XMLPOT): $(XMLFILES) $(TRANSLATIONITS)
	@echo "ITSTOOL $@"
	cat $(XMLFILES) | sed 's@^@$(top_srcdir)/@' | tr '\n' '\0' | tr -d '\r' | xargs -0 $(ITSTOOL) -i $(TRANSLATIONITS) > $@
else # HAVE_ITSTOOL
$(XMLPOT): $(XMLFILES) $(TRANSLATIONITS)
	@echo "XGETTEXT $@"
	$(XGETTEXT) --from-code=UTF-8 --its=$(TRANSLATIONITS) -f $(XMLFILES) -D $(top_srcdir) -o $@
endif # HAVE_ITSTOOL

if HAVE_DATA
pot: $(CREDITSPOT) $(XMLPOT) $(LANGUAGEFILE)
	$(MAKE) -C po
	$(MAKE) -C po naev_src.pot
	$(XGETTEXT) --from-code=UTF-8 -o $(NAEVPOT) $(NAEVSRCPOT) $(XMLPOT) $(CREDITSPOT)
	$(top_srcdir)/utils/gettext.sh $(top_builddir)

$(LANGUAGEFILE): $(top_srcdir)/po/LINGUAS
	@mkdir -p $(top_builddir)/dat
	tail -n +2 $< > $@
else # HAVE_DATA
pot:
endif # HAVE_DATA

else # HAVE_NLS
pot:
	@echo "***"
	@echo "*** You must have NLS enabled to compile translation files"
	@echo "***"
endif # HAVE_NLS

$(NAEV):
	@echo "  LN    $(NAEV)"
	$(LN_S) "src/$(NAEV)" "$(NAEV)"

$(VERSIONFILE): src/config.h
	@mkdir -p $(top_builddir)/dat
	@echo $(VERSION) > $@

utils: $(MKSPR)

$(MKSPR):
	$(MAKE) -C utils/mkspr

install-data-local:
	(cd $(top_srcdir)/dat; find . -type f -exec install -Dm 644 "{}" "$(DESTDIR)$(pkgdatadir)/dat/{}" \;)

uninstall-hook:
	echo " rm -rf '$(DESTDIR)$(pkgdatadir)'"; \
	echo " rm -rf '$(docdir)'"; \
	rm -rf "$(DESTDIR)$(pkgdatadir)"
	rm -rf "$(docdir)"

docs:
	$(MAKE) -C docs

soundtrack: naev-$(VERSION)-soundtrack.zip

naev-$(VERSION)-soundtrack.zip: $(top_srcdir)/utils/soundtrack.py dat/snd/soundtrack.yaml $(VERSIONFILE)
	$(top_srcdir)/utils/soundtrack.py --source-dir=$(top_srcdir)

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
