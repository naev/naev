SUBDIRS = lib src utils

dist_man6_MANS = naev.6

DATA_ARCHIVE = ndata
PACK	     = utils/pack/pack

EXTRA_DIST = LICENSE conf.example
CLEANFILES = pack $(DATA_ARCHIVE)

V             = @
Q             = $(V:1=)
QUIET_PACK    = $(Q:@=@echo ' PACK   '$@;)

.PHONY: docs help install-ndata

$(PACK):
	$(MAKE) -C utils/pack

VERSION:
	echo $(VERSION) > $@

$(DATA_ARCHIVE): $(PACK) VERSION
	$(QUIET_PACK)\
	test -f $(top_srcdir)/VERSION || mv -n VERSION "$(top_srcdir)"; \
	cd "$(top_srcdir)" && \
	$(abs_top_builddir)/$(PACK) "$(abs_top_builddir)/$(DATA_ARCHIVE)" \
	  AUTHORS VERSION \
	  $$(find ai      -type f -name '*.lua'; \
	     find gfx     -type f -name '*.png'; \
	     find snd     -type f \
	          \( -name '*.ogg' -o -name '*.wav' -o -name '*.lua' \); \
	     find scripts -type f -name '*.lua'; \
	     find dat     -type f); \
	test -f $(abs_top_builddir)/VERSION \
	  || mv -n VERSION "$(abs_top_builddir)"

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
	rm -rf "$(DESTDIR)$(pkgdatadir)"

docs:
	$(MAKE) -C docs

help:
	@echo "Possible targets are:"
	@echo "       all - builds everything"
	@echo "     ndata - creates the ndata file"
	@echo "      docs - creates the doxygen documentation"
	@echo "     clean - removes binaries and object files"
	@echo " distclean - does clean and removes files created by configure"
	@echo "   install - installs naev"
	@echo " uninstall - removes previously installed files"
