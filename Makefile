
DEBUG = 1

APPNAME = main


VMAJOR = 0
VMINOR = 0
VREV = 1
VERSION = -DVMAJOR=$(VMAJOR) -DVMINOR=$(VMINOR) -DVREV=$(VREV)

OBJS := $(shell find src/ -name '*.c' -print)
OBJS := $(OBJS:%.c=%.o)

CLUA = -Ilib/lua
CSDL = $(shell sdl-config --cflags)
CXML = $(shell xml2-config --cflags)
CTTF = $(shell freetype-config --cflags)
CGL = 
CFLAGS = $(CLUA) $(CSDL) $(CXML) $(CTTF) $(CGL) $(VERSION)
ifdef DEBUG
CFLAGS += -W -Wall -g3 -DDEBUG -DLUA_USE_APICHECK
else # DEBUG
CFLAGS += -O2
endif # DEBUG

LDLUA = lib/lua/liblua.a
LDSDL = `sdl-config --libs` -lSDL_image
LDXML = `xml2-config --libs`
LDTTF = `freetype-config --libs`
LDGL = -lGL
LDFLAGS = -lm $(LDLUA) $(LDSDL) $(LDXML) $(LDTTF) $(LDGL)

DATA = data
DATAFILES = $(shell find ai/ gfx/ dat/ -name '*.lua' -o -name '*.png' -o -name '*.xml' -o -name '*.ttf')


%.o:	%.c %.h
	@$(CC) -c $(CFLAGS) -o $@ $<
	@echo -e "\tCC   $@"


all:	data lua $(OBJS)
	@$(CC) $(LDFLAGS) -o $(APPNAME) $(OBJS) lib/lua/liblua.a
	@echo -e "\tLD   $(APPNAME)"


lua:
	@if [ ! -e lib/lua/lua ];then ( cd lib/lua; $(MAKE) a ); fi

pack: src/pack.c utils/pack/main.c
	@( cd utils/pack; $(MAKE) )


data: pack $(DATAFILES) src/pack.c utils/pack/main.c
	@echo -e "\tCreating data\n"
	@./pack $(DATA) $(DATAFILES)


clean:
	@echo -e "\tRemoving data"
	@rm -f $(DATA)
	@echo -e "\tRemoving object files"
	@rm -f $(OBJS)


