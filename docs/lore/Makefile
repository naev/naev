
SPOB := $(wildcard ../../dat/spob/*.xml)
SPOB := $(patsubst ../../dat/spob/%.xml, content/spob/%.md, $(SPOB) )

GFX := $(wildcard ../../artwork/gfx/spob/space/*.webp)
GFX := $(patsubst ../../artwork/%, content/%, $(GFX))

.PHONY: all gfx spob view install check logo

all: logo gfx spob
	bundle exec nanoc

content/spob/%.md: ../../dat/spob/%.xml
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

view: all
	bundle exec nanoc view

install: all
	test -d naev.github.io || git clone git@github.com:naev/naev.github.io.git
	rsync -avzh --delete --exclude=".*" --exclude="CNAME" output/ naev.github.io
	(cd naev.github.io && git add . && git commit -a -m "updates" && git push)

check: all
	bundle exec nanoc check
