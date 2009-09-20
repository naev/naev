SUBDIRS = lib src

dist_man6_MANS = naev.6

DATA_ARCHIVE = ndata

EXTRA_DIST = LICENSE conf.example
CLEANFILES = pack $(DATA_ARCHIVE)

V             = @
Q             = $(V:1=)
QUIET_PACK    = $(Q:@=@echo ' PACK   '$@;)

.PHONY: install-ndata

pack:
	$(MAKE) -C utils/pack

VERSION:
	echo $(VERSION) > $@

$(DATA_ARCHIVE): pack VERSION
	$(QUIET_PACK)./pack $(DATA_ARCHIVE) AUTHORS VERSION \
	     $$(find ai      -type f -name '*.lua'; \
		find gfx     -type f -name '*.png'; \
		find snd     -type f \( -name '*.ogg' -o -name '*.wav' \); \
		find scripts -type f -name '*.lua'; \
		find dat     -type f); \
	rm -f VERSION

install-ndata: $(DATA_ARCHIVE)
	ndata="$(DESTDIR)$(ndata_path)"; \
	dir=`echo "$$ndata" | sed 's/\(.*\)[\/\\].*/\1/'`; \
	echo " $(MKDIR_P) $$dir"; \
	$(MKDIR_P) "$$dir"; \
	echo " $(INSTALL_DATA) -c $(DATA_ARCHIVE) $$ndata"; \
	$(INSTALL_DATA) -c "$(DATA_ARCHIVE)" "$$ndata"

install-data-hook:
	if test -e dat && test -e gfx; then \
	  $(MAKE) install-ndata; \
	fi
