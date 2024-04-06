
SPOB := $(wildcard ../../dat/spob/*.xml)
SPOB := $(patsubst ../../dat/spob/%.xml, content/spob/%.md, $(SPOB) )

SSYS := $(wildcard ../../dat/ssys/*.xml)
SSYS := $(patsubst ../../dat/ssys/%.xml, content/ssys/%.md, $(SSYS) )

FCTS := $(wildcard ../../dat/factions/*.xml)
FCTS := $(patsubst ../../dat/factions/%.xml, content/fcts/%.md, $(FCTS) )

GFX := $(wildcard ../../artwork/gfx/spob/**/*.webp ../../artwork/gfx/spob/space/**/*.webp ../../artwork/gfx/logo/*.webp)
GFX := $(patsubst ../../artwork/%, content/%, $(GFX))

.PHONY: all gfx spob ssys fcts view clean check logo

all: logo gfx spob ssys fcts
	bundle exec nanoc

content/spob/%.md: ../../dat/spob/%.xml
	@mkdir -p $(dir $@)
	@echo "---" > $@
	yq -oy  '.' $< >> $@
	@echo "---" >> $@

content/ssys/%.md: ../../dat/ssys/%.xml
	@mkdir -p $(dir $@)
	@echo "---" > $@
	yq -oy  '.' $< >> $@
	@echo "---" >> $@

content/fcts/%.md: ../../dat/factions/%.xml
	@mkdir -p $(dir $@)
	@echo "---" > $@
	yq -oy  '.' $< >> $@
	@echo "---" >> $@

content/gfx/%.webp: ../../artwork/gfx/%.webp
	@mkdir -p $(dir $@)
	cp $< $@

logo: content/imgs/naev.png content/favicon.png

content/imgs/naev.png: ../../extras/logos/naev.png
	@mkdir -p $(dir $@)
	cp $< $@

content/favicon.png: ../../extras/logos/naev.png
	@mkdir -p $(dir $@)
	cp $< $@

gfx: $(GFX)

spob: $(SPOB)

ssys: $(SSYS)

fcts: $(FCTS)

view: all
	bundle exec nanoc view


check: all
	bundle exec nanoc check

clean:
	rm -rf  content/gfx/ content/spob/ content/ssys/ content/fcts/ content/favicon.png content/imgs/naev.png
