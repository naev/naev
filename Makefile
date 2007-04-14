
DEBUG = 1

APPNAME = main

VERSION = -DVMAJOR=0 -DVMINOR=0 -DVREV=1

OBJS := $(shell find src/ -name '*.c' -print)
OBJS := $(OBJS:%.c=%.o)

CLUA = -I/usr/include/lua5.1
CSDL = `sdl-config --cflags`
CXML = `xml2-config --cflags`
CTTF = `freetype-config --cflags`
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
DATAFILES = $(shell find ai/ gfx/ dat/ -name '*.lua' -o -name '*.png' -o -name '*.xml' -print)


%.o:	%.c
	@$(CC) -c $(CFLAGS) -o $@ $<
	@echo -e "\tCC   $@"


all:	data $(OBJS)
	@$(CC) $(LDFLAGS) -o $(APPNAME) $(OBJS)
	@echo -e "\tLD   $(APPNAME)"


data: $(DATAFILES)
	@echo -e "\tCreating data\n"
	@ls -1 $(DATAFILES) | cpio --quiet -o > $(DATA)


clean:
	@echo -e "\tRemoving data"
	@rm -f $(DATA)
	@echo -e "\tRemoving object files"
	@rm -f $(OBJS)


