SUBDIRS = lib src

dist_man6_MANS = naev.6

DATA_ARCHIVE = ndata-$(VERSION)

EXTRA_DIST = LICENSE conf.example
CLEANFILES = pack $(DATA_ARCHIVE)

V          = @
Q          = $(V:1=)
QUIET_PACK = $(Q:@=@echo ' PACK   '$@;)

pack:
	cd utils/pack && make

VERSION:
	echo $(VERSION) > $@

ndata: $(DATA_ARCHIVE)

$(DATA_ARCHIVE): pack VERSION
	$(QUIET_PACK)./pack $(DATA_ARCHIVE) AUTHORS VERSION \
	     $$(find ai      -type f -name '*.lua'; \
		find gfx     -type f -name '*.png'; \
		find snd     -type f \( -name '*.ogg' -o -name '*.wav' \); \
		find scripts -type f -name '*.lua'; \
		find dat     -type f); \
	rm -f VERSION
