
SPOB := $(wildcard ../../dat/spob/*.xml)
SPOB := $(patsubst ../../dat/spob/%.xml, content/spob/%.md, $(SPOB) )

SSYS := $(wildcard ../../dat/ssys/*.xml)
SSYS := $(patsubst ../../dat/ssys/%.xml, content/ssys/%.md, $(SSYS) )

FCTS := $(shell find ../../dat/factions -type f -name "*.xml" -exec grep -vqz "<invisible />" {} \; -print)
FCTS := $(patsubst ../../dat/factions/%.xml, content/fcts/%.md, $(FCTS) )

MISN := $(shell find ../../dat/missions -type f -name "*.lua" -exec grep -q "<?xml version=['\"]1.0['\"] encoding=['\"]utf8['\"]?>" {} \; -print)
MISN := $(patsubst ../../dat/missions/%.lua, content/misn/%.lua, $(MISN) )

CMPN := $(shell find ../../dat/missions -type f -name "readme.md")
CMPN := $(patsubst ../../dat/missions/%/readme.md, content/cmpn/%.md, $(CMPN))

EVTS:= $(shell find ../../dat/events -type f -name "*.lua" -exec grep -q "<?xml version=['\"]1.0['\"] encoding=['\"]utf8['\"]?>" {} \; -print)
EVTS := $(patsubst ../../dat/events/%.lua, content/evts/%.lua, $(EVTS) )

GFX := $(wildcard ../../artwork/gfx/spob/**/*.webp ../../artwork/gfx/spob/space/**/*.webp ../../artwork/gfx/logo/*.webp ../../artwork/gfx/vn/characters/*.webp ../../artwork/gfx/portraits/*.webp ../../artwork/gfx/vn/characters/*.png ../../artwork/gfx/portraits/*.png)
GFX := $(patsubst ../../artwork/%, content/%, $(GFX))

.PHONY: all gfx spob ssys fcts misn cmpn evts view install check logo

all: logo gfx spob ssys fcts misn cmpn evts
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

content/cmpn/%.md: ../../dat/missions/%/readme.md
	@mkdir -p $(dir $@)
	cp $< $@

content/evts/%.lua: ../../dat/events/%.lua
	@mkdir -p $(dir $@)
	@echo "---" > $@
	@sed -n '/--\[\[/{:a;n;/--\]\]/q;p;ba}' $< | yq -oy '.' -p xml >> $@
	@echo "---" >> $@
	cat $< >> $@

content/gfx/%: ../../artwork/gfx/%
	@mkdir -p $(dir $@)
	cp $< $@

logo: content/imgs/naev.png content/favicon.png

content/imgs/naev.png: ../../artwork/gfx/Naev.webp
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

cmpn: $(CMPN)

evts: ${EVTS}

view: all
	bundle exec nanoc view

check: all
	bundle exec nanoc check

clean:
	rm -rf content/imgs/naev.png content/favicon.png content/gfx content/evts content/misn content/cmpn content/fcts content/ssys content/spob tmp output
