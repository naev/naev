
DEBUG = 1

APPNAME = main

OBJS := main.o		\
		physics.o	\
		opengl.o		\
		ship.o		\
		pilot.o

CFLAGS = -Wall `sdl-config --cflags` `xml2-config --cflags`
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
	@gcc -c $(CFLAGS) -o $@ $<
	@echo -e "\tCC   $@"


all:	data $(OBJS)
	@gcc $(LDFLAGS) -o $(APPNAME) $(OBJS)
	@echo -e "\tLD   $(APPNAME)"



data_init:
	@echo -e '<?xml version="1.0" encoding="UTF-8"?>\n<Data>' > data
data: data_init $(DOBJS)
	@echo -e '</Data>' >> data
	@echo -e "\tCreating data\n"


clean:
	rm -f $(OBJS)
