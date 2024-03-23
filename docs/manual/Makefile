TEX := lualatex -shell-escape

.PHONY: all naev_dev_manual.pdf

all: naev_dev_manual.pdf llama

version.tex: ../../dat/VERSION
	echo -n "\version{" > $@
	tr -d "\n" < $< >> $@
	echo "}" >> $@

naev_dev_manual.pdf: naev_dev_manual.tex Naev.png llama version.tex
	latexmk -pdf -pdflatex="$(TEX)" naev_dev_manual

Naev.png: ../../artwork/gfx/Naev.webp
	gm convert "$<" "$@"

llama: images/llama.png images/llamaengine.png images/llamacomm.png

images/llama.png: ../../artwork/gfx/ship/llama/llama.webp
	test -d images || mkdir images
	gm convert "$<" "$@"

images/llamaengine.png: ../../artwork/gfx/ship/llama/llama_engine.webp
	test -d images || mkdir images
	gm convert "$<" "$@"

images/llamacomm.png: ../../artwork/gfx/ship/llama/llama_comm.webp
	test -d images || mkdir images
	gm convert "$<" "$@"

clean:
	$(RM) naev_dev_manual.aux naev_dev_manual.fdb_latexmk naev_dev_manual.fls naev_dev_manual.log naev_dev_manual.lol naev_dev_manual.markdown.lua naev_dev_manual.markdown.out naev_dev_manual.out naev_dev_manual.toc
