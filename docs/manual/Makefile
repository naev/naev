TEX := lualatex -shell-escape

.PHONY: all naev_dev_manual.pdf

all: naev_dev_manual.pdf

naev_dev_manual.pdf: naev_dev_manual.tex Naev.png
	latexmk -pdf -pdflatex="$(TEX)" naev_deV_manual

Naev.png: ../../artwork/gfx/Naev.webp
	convert "$<" "$@"
