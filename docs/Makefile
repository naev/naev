

.PHONY: all lua doxy

all: doxy lua

doxy:
	@echo "   DOC  docs/doxygen"
	@(cd ../src; doxygen)

lua:
	@echo "   DOC  docs/lua"
	@sh luadoc.sh
