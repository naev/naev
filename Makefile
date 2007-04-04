
DEBUG = 1

APPNAME = main

VERSION = -DVMAJOR=0 -DVMINOR=0 -DVREV=1

OBJS := main.o		\
		physics.o	\
		opengl.o		\
		ship.o		\
		pilot.o		\
		player.o		\
		joystick.o	\
		space.o		\
		rng.o

CLUA = -I/usr/include/lua5.1
CSDL = `sdl-config --cflags`
CXML = `xml2-config --cflags`
CGL = 
CFLAGS = -Wall $(CLUA) $(CSDL) $(CXML) $(CGL) $(VERSION)
ifdef DEBUG
CFLAGS += -g3 -DDEBUG
else # DEBUG
CFLAGS += -O2
endif # DEBUG

LDLUA = -llua5.1
LDSDL = `sdl-config --libs` -lSDL_image
LDXML = `xml2-config --libs`
LDGL = -lGL
LDFLAGS = -lm $(LDLUA) $(LDSDL) $(LDXML) $(LDGL)


DOBJS = ship.xml	\
		faction.xml	\
		fleet.xml	\
		outfit.xml	\
		planet.xml	\
		ssys.xml


%.xml:
	@sed -e '/^<?xml.*/d' dat/$@ >> data
%.o:	%.c
	@$(CC) -c $(CFLAGS) -o $@ $<
	@echo -e "\tCC   $@"


all:	data $(OBJS)
	@$(CC) $(LDFLAGS) -o $(APPNAME) $(OBJS)
	@echo -e "\tLD   $(APPNAME)"



data_init:
	@echo -e '<?xml version="1.0" encoding="UTF-8"?>\n<Data>' > data
data: data_init $(DOBJS)
	@echo -e '</Data>' >> data
	@echo -e "\tCreating data\n"


clean:
	rm -f $(OBJS)
