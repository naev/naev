
DEBUG = 1

APPNAME = main

OBJS := main.o		\
		physics.o	\
		opengl.o

CFLAGS = -Wall `sdl-config --cflags`
ifdef DEBUG
CFLAGS += -g3 -DDEBUG
else # DEBUG
CFLAGS += -O2
endif # DEBUG

LDFLAGS = -lm `sdl-config --libs` -lSDL_image -lGL


%.o:	%.c
	gcc -c $(CFLAGS) -o $@ $<


all:	$(OBJS)
	gcc $(LDFLAGS) -o $(APPNAME) $(OBJS)


clean:
	rm -f $(OBJS)
