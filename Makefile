
DEBUG = 1

APPNAME = main


VMAJOR = 0
VMINOR = 0
VREV = 1
VERSION = -DVMAJOR=$(VMAJOR) -DVMINOR=$(VMINOR) -DVREV=$(VREV)

OBJS := $(shell find src/ -name '*.c' -print)
OBJS := $(OBJS:%.c=%.o)

CLUA = -I/usr/include/lua5.1
CSDL = $(shell sdl-config --cflags)
CXML = $(shell xml2-config --cflags)
CTTF = $(shell freetype-config --cflags)
CGL = 
CFLAGS = -Wall $(CLUA) $(CSDL) $(CXML) $(CTTF) $(CGL) $(VERSION)
ifdef DEBUG
CFLAGS += -g3 -DDEBUG -DLUA_USE_APICHECK
else # DEBUG
CFLAGS += -O2
endif # DEBUG

LDLUA = -llua5.1
LDSDL = `sdl-config --libs` -lSDL_image
LDXML = `xml2-config --libs`
LDTTF = `freetype-config --libs`
LDGL = -lGL
LDFLAGS = -lm $(LDLUA) $(LDSDL) $(LDXML) $(LDTTF) $(LDGL)

DATA = data
DATAFILES = $(shell find ai/ gfx/ dat/ -name '*.lua' -o -name '*.png' -o -name '*.xml')


%.o:	%.c %.h
	@$(CC) -c $(CFLAGS) -o $@ $<
	@echo -e "\tCC   $@"


all:	data $(OBJS)
	@$(CC) $(LDFLAGS) -o $(APPNAME) $(OBJS)
	@echo -e "\tLD   $(APPNAME)"


pack: src/pack.c
	@( cd utils/pack; $(MAKE) )


data: pack $(DATAFILES) src/pack.c
	@echo -e "\tCreating data\n"
	@./pack $(DATA) $(DATAFILES)


clean:
	@echo -e "\tRemoving data"
	@rm -f $(DATA)
	@echo -e "\tRemoving object files"
	@rm -f $(OBJS)


