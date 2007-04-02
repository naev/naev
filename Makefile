
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

CFLAGS = -Wall `sdl-config --cflags` `xml2-config --cflags` $(VERSION)
ifdef DEBUG
CFLAGS += -g3 -DDEBUG
else # DEBUG
CFLAGS += -O2
endif # DEBUG

LDFLAGS = -lm `sdl-config --libs` `xml2-config --libs` -lSDL_image -lGL


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
