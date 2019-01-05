.PHONY: doxy lua

all-local: doxy lua

if HAVE_DOXYGEN
doxy:
	@echo "   DOC  docs/doxygen"
	@(cd ../src; doxygen)
else
doxy:
	@echo "***"
	@echo "*** doxygen missing -- not generating C documentation"
	@echo "***"
endif

if HAVE_LDOC
lua:
	@echo "   DOC  docs/lua"
	@sh luadoc.sh
else
lua:
	@echo "***"
	@echo "*** luadoc missing -- not generating lua documentation"
	@echo "***"
endif
