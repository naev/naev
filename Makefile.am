ACLOCAL_AMFLAGS = --install -I m4

SUBDIRS = lib src
DIST_SUBDIRS = lib src

if HAVE_UTILS
SUBDIRS += utils
endif

appicondir = $(datadir)/pixmaps
appicon_DATA = extras/logos/naev.png
Graphicsdir = $(datadir)/applications
Graphics_DATA = naev.desktop
dist_pkgdata_DATA = naev-confupdate.sh

dist_man6_MANS = naev.6
dist_doc_DATA = AUTHORS README TODO LICENSE

DATA_ARCHIVE = ndata
PACK	     = utils/pack/pack$(EXEEXT)
if HAVE_UTILS
MKSPR      = utils/mkspr/mkspr$(EXEEXT)
endif
NAEV	     = naev$(EXEEXT)

EXTRA_DIST = LICENSE extras naev.desktop
CLEANFILES = $(DATA_ARCHIVE) $(NAEV)

.PHONY: docs utils help install-ndata

all-local: $(NAEV) VERSION

$(NAEV):
	@echo "  LN    $(NAEV)"
	$(LN_S) "src/$(NAEV)" "$(NAEV)"

$(PACK):
	$(MAKE) -C utils/pack

VERSION: src/config.h
	echo $(VERSION) > $@

if HAVE_UTILS
utils: $(PACK) $(MKSPR)

$(MKSPR):
	$(MAKE) -C utils/mkspr

$(DATA_ARCHIVE): $(PACK) VERSION
	@echo " PACK   $(DATA_ARCHIVE)"
	cd "$(top_srcdir)" && \
	  (find AUTHORS VERSION naev-confupdate.sh -print0; \
	     find ai      -type f -name '*.lua' -print0; \
	     find gfx     -type f -name '*.png' -print0; \
	     find snd     -type f \
	          \( -name '*.ogg' -o -name '*.wav' -o -name '*.lua' \) -print0; \
	     find scripts -type f -name '*.lua' -print0; \
	     find dat     -type f -print0) | xargs -0 \
	$(abs_top_builddir)/$(PACK) "$(abs_top_builddir)/$(DATA_ARCHIVE)";
else
utils:

$(DATA_ARCHIVE):
	@echo "***"
	@echo "*** You must clone the full git repo to create $(DATA_ARCHIVE)"
	@echo "***"
	@false
endif

install-ndata: $(DATA_ARCHIVE)
	ndata="$(DESTDIR)$(ndata_path)"; \
	dir=$$(echo "$$ndata" | sed 's/\(.*\)[\/\\].*/\1/'); \
	echo " $(MKDIR_P) '$$dir'"; \
	$(MKDIR_P) "$$dir"; \
	echo " $(INSTALL_DATA) -c '$(DATA_ARCHIVE)' '$$ndata'"; \
	$(INSTALL_DATA) -c "$(DATA_ARCHIVE)" "$$ndata"

install-data-hook:
	if test -e $(top_srcdir)/dat && test -e $(top_srcdir)/gfx; then \
	  $(MAKE) install-ndata; \
	fi

uninstall-hook:
	echo " rm -rf '$(DESTDIR)$(pkgdatadir)'"; \
	echo " rm -rf '$(docdir)'"; \
	rm -rf "$(DESTDIR)$(pkgdatadir)"
	rm -rf "$(docdir)"

docs:
	$(MAKE) -C docs

help:
	@echo "Possible targets are:"
	@echo "        all - builds everything"
	@echo "      ndata - creates the ndata file"
	@echo "       docs - creates the doxygen documentation"
	@echo "      clean - removes binaries and object files"
	@echo "    install - installs naev"
	@echo "  uninstall - removes previously installed files"
	@echo " dist-bzip2 - creates a source tarball"
	@echo "  distcheck - checks if the created tarball works properly"
	@echo "  distclean - does clean and removes files created by configure"
