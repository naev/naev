#
#   OPTIONS
#
DEBUG := 1
#DEBUG_PARANOID := 1


# OS
OS := LINUX
#OS := WIN32
#OS := MACOS
export OS


#
#   VERSION
#
VMAJOR      := 0
VMINOR      := 0
VREV        := 1
VERSION     := -DVMAJOR=$(VMAJOR) -DVMINOR=$(VMINOR) -DVREV=$(VREV)
VERSIONFILE := VERSION


#
#   OBJS
#
APPNAME := naev
OBJS    := $(patsubst %.c,%.o,$(wildcard src/*.c))


#
#   CFLAGS
#
CLUA   := -Ilib/lua
CSDL   := $(shell sdl-config --cflags)
CXML   := $(shell xml2-config --cflags)
CTTF   := $(shell freetype-config --cflags)
CGL    :=
CFLAGS := $(CLUA) $(CSDL) $(CXML) $(CTTF) $(CGL) $(VERSION) -D$(OS)
ifeq ($(OS),LINUX)
CFLAGS += -D_POSIX_SOURCE
endif
ifdef DEBUG
CFLAGS += -W -Wall -Wextra -Wunused -Wshadow -Wpointer-arith -Wmissing-prototypes -Winline -Wcast-align -Wmissing-declarations -fstack-protector -fstack-protector-all -g3 -DDEBUG -DLUA_USE_APICHECK -std=c99 -ansi
ifdef DEBUG_PARANOID
CFLAGS += -DDEBUG_PARANOID
endif # DEBUG_PARANOID
else # DEBUG
CFLAGS += -O2 -funroll-loops -pipe -std=c99 -ansi
endif # DEBUG


#
#   LDFLAGS
#
LDLUA   := lib/lua/liblua.a
LDSDL   := $(shell sdl-config --libs) -lSDL_image -lSDL_mixer
LDXML   := $(shell xml2-config --libs)
LDTTF   := $(shell freetype-config --libs)
LDGL    := -lGL
LDFLAGS := -lm $(LDLUA) $(LDSDL) $(LDXML) $(LDTTF) $(LDGL)


#
#   DATA
#
DATA_AI   := 	$(wildcard ai/*.lua \
					ai/include/*.lua \
					ai/tpl/*.lua )
DATA_GFX  := 	$(wildcard gfx/*.png \
					gfx/gui/*.png \
					gfx/logo/*.png \
					gfx/outfit/space/*.png \
					gfx/outfit/store/*.png \
					gfx/planet/exterior/*.png \
					gfx/planet/space/*.png \
					gfx/ship/*.png \
					gfx/spfx/*.png)
DATA_XML  := 	$(wildcard dat/*.xml dat/*.ttf)
DATA_SND  := 	$(wildcard snd/music/*.ogg snd/sounds/*.wav) snd/music.lua
DATA_MISN := 	$(wildcard dat/missions/*.lua)
DATA      := 	data
DATAFILES := 	$(DATA_AI) $(DATA_GFX) $(DATA_XML) $(DATA_SND) $(DATA_MISN)


#
#   TARGETS
#
.PHONY: all clean purge


%.o:	%.c %.h
	@$(CC) -c $(CFLAGS) -o $@ $<
	@echo -e "\tCC   $@"


all:	utils data lua naev


help:
	@echo "Possible targets are:":
	@echo "   lua - makes Lua"
	@echo "   naev - makes the naev binary"
	@echo "   mkpsr - makes the mkspr utilitily"
	@echo "   data - creates the data file"
	@echo "   utils - makes all the utilities"
	@echo "   docs - creates the doxygen documentation"
	@echo "   clean - removes naev's main binary and data file"
	@echo "   purge - removes everything done"


naev: $(OBJS)
	@$(CC) $(LDFLAGS) -o $(APPNAME) $(OBJS) lib/lua/liblua.a
	@echo -e "\tLD   $(APPNAME)"


lua:
	+@if [ ! -e lib/lua/liblua.a ];then make -C lib/lua a; fi


pack: src/md5.c src/pack.c utils/pack/main.c
	+@make -C utils/pack


mkspr: utils/mkspr/main.c
	+@make -C utils/mkspr


$(VERSIONFILE):
	@echo -n "$(VMAJOR).$(VMINOR).$(VREV)" > $(VERSIONFILE)


data: pack $(DATAFILES)
	@echo -n "$(VMAJOR).$(VMINOR).$(VREV)" > $(VERSIONFILE)
	@echo -e "\tCreating data\n"
	@./pack $(DATA) $(DATAFILES) $(VERSIONFILE)


utils: pack mkspr


docs:
	@( cd src; doxygen )


clean:
	@echo -e "\tRemoving data"
	@$(RM) $(DATA)
	@echo -e "\tRemoving object files"
	@$(RM) $(OBJS)


purge: clean
	@echo -e "\tCleaning utilities"
	@make -C utils/pack clean
	@make -C utils/mkspr clean
	@echo -e "\tCleaning Lua"
	@make -C lib/lua clean


