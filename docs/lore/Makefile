
SPOB := $(wildcard ../../dat/spob/*.xml)
SPOB := $(patsubst ../../dat/spob/%.xml, content/spob/%.md, $(SPOB) )

SSYS := $(wildcard ../../dat/ssys/*.xml)
SSYS := $(patsubst ../../dat/ssys/%.xml, content/ssys/%.md, $(SSYS) )

FCTS := $(wildcard ../../dat/factions/*.xml)
FCTS := $(patsubst ../../dat/factions/%.xml, content/fcts/%.md, $(FCTS) )

MISN := $(shell find ../../dat/missions -type f -name "*.lua" -exec grep -q "<?xml version=['\"]1.0['\"] encoding=['\"]utf8['\"]?>" {} \; -print)
MISN := $(patsubst ../../dat/missions/%.lua, content/misn/%.lua, $(MISN) )

EVTS:= $(shell find ../../dat/events -type f -name "*.lua" -exec grep -q "<?xml version=['\"]1.0['\"] encoding=['\"]utf8['\"]?>" {} \; -print)
EVTS := $(patsubst ../../dat/events/%.lua, content/evts/%.lua, $(EVTS) )

GFX := $(wildcard ../../artwork/gfx/spob/**/*.webp ../../artwork/gfx/spob/space/**/*.webp ../../artwork/gfx/logo/*.webp)
GFX := $(patsubst ../../artwork/%, content/%, $(GFX))

.PHONY: all gfx spob ssys fcts misn evts view install check logo

all: logo gfx spob ssys fcts misn evts
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

content/misn/%.lua: ../../dat/missions/%.lua
	@mkdir -p $(dir $@)
	@echo "---" > $@
	@sed -n '/--\[\[/{:a;n;/--\]\]/q;p;ba}' $< | yq -oy '.' -p xml >> $@
	@echo "---" >> $@
	cat $< >> $@

content/evts/%.lua: ../../dat/events/%.lua
	@mkdir -p $(dir $@)
	@echo "---" > $@
	@sed -n '/--\[\[/{:a;n;/--\]\]/q;p;ba}' $< | yq -oy '.' -p xml >> $@
	@echo "---" >> $@
	cat $< >> $@

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

misn: $(MISN)

evts: ${EVTS}

view: all
	bundle exec nanoc view

check: all
	bundle exec nanoc check

clean:
	rm -rf content/imgs/naev.png content/favicon.png content/gfx content/evts content/misn content/fcts content/ssys content/spob tmp output
