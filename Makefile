#
#   OPTIONS
#
DEBUG = 1

# OS
OS := LINUX
#OS := WIN32


#
#   VERSION
#
VMAJOR = 0
VMINOR = 0
VREV = 1
VERSION = -DVMAJOR=$(VMAJOR) -DVMINOR=$(VMINOR) -DVREV=$(VREV)
VERSIONFILE = VERSION


#
#   OBJS
#
APPNAME = naev
OBJS := $(shell find src/ -name '*.c' -print)
OBJS := $(OBJS:%.c=%.o)


#
#   CFLAGS
#
CLUA = -Ilib/lua
CSDL = $(shell sdl-config --cflags)
CXML = $(shell xml2-config --cflags)
CTTF = $(shell freetype-config --cflags)
CAL = $(shell openal-config --cflags) $(shell freealut-config --cflags)
CVORBIS =
CGL =
CFLAGS = $(CLUA) $(CPLUTO) $(CSDL) $(CXML) $(CTTF) $(CGL) $(CAL) $(CVORBIS) $(VERSION) -D$(OS) -fgnu89-inline
ifdef DEBUG
CFLAGS += -W -Wall -Wextra -Wmissing-prototypes -Winline -Wcast-align -Wmissing-declarations -fno-inline -g3 -DDEBUG -DLUA_USE_APICHECK -std=c99 -ansi
else # DEBUG
CFLAGS += -O2 -funroll-loops -pipe -std=c99 -ansi
endif # DEBUG


#
#   LDFLAGS
#
LDLUA = lib/lua/liblua.a
LDSDL = $(shell sdl-config --libs) -lSDL_image
LDXML = $(shell xml2-config --libs)
LDTTF = $(shell freetype-config --libs)
LDGL = -lGL
LDAL = $(shell openal-config --libs) $(shell freealut-config --libs)
LDVORBIS = -lvorbisfile
LDFLAGS = -lm $(LDLUA) $(LDPLUTO) $(LDSDL) $(LDXML) $(LDTTF) $(LDGL) $(LDAL) $(LDVORBIS)


#
#   DATA
#
DATA_AI = $(shell find ai/ -name '*.lua')
DATA_GFX = $(shell find gfx/ -name '*png')
DATA_XML = $(shell find dat/ -name '*.xml' -o -name '*.ttf')
DATA_SND = $(shell find snd/ -name '*.ogg' -o -name '*.wav') snd/music.lua
DATA_MISN = $(shell find dat/missions/ -name '*.lua')
DATA = data
DATAFILES = $(VERSIONFILE) $(DATA_AI) $(DATA_GFX) $(DATA_XML) $(DATA_SND) $(DATA_MISN)



#
#   TARGETS
#
%.o:	%.c %.h
	@$(CC) -c $(CFLAGS) -o $@ $<
	@echo -e "\tCC   $@"


all:	utils data lua $(OBJS)
	@$(CC) $(LDFLAGS) -o $(APPNAME) $(OBJS) lib/lua/liblua.a
	@echo -e "\tLD   $(APPNAME)"


lua:
	@if [ ! -e lib/lua/liblua.a ];then ( cd lib/lua; $(MAKE) a ); fi

pack: src/pack.c utils/pack/main.c
	@( cd utils/pack; $(MAKE) )

mkspr: utils/mkspr/main.c
	@( cd utils/mkspr; $(MAKE) )


$(VERSIONFILE):
	@echo -n "$(VMAJOR).$(VMINOR).$(VREV)" > $(VERSIONFILE)


data: pack $(DATAFILES) src/pack.c utils/pack/main.c
	@echo -n "$(VMAJOR).$(VMINOR).$(VREV)" > $(VERSIONFILE)
	@echo -e "\tCreating data\n"
	@./pack $(DATA) $(DATAFILES)

utils: mkspr


clean:
	@echo -e "\tRemoving data"
	@rm -f $(DATA)
	@echo -e "\tRemoving object files"
	@rm -f $(OBJS)

purge: clean
	@echo -e "\tCleaning utilities"
	@( cd utils/pack; $(MAKE) clean )
	@( cd utils/mkspr; $(MAKE) clean )
	@echo -e "\tCleaning Lua"
	@( cd lib/lua; $(MAKE) clean )


