
ROOTDIR = ../../
LOCALDIR = utils/mksprite/

COBJS =

CFLAGS = `sdl-config --cflags` -O2 -Wall
LFLAGS = `sdl-config --libs` -lSDL_image

APPNAME = mkspr

%.o: $(SRCDIR)%.c
	@$(CC) -c $(CFLAGS) -o $@ $<
	@echo -e "\tCC   $(LOCALDIR)$@"

all: $(COBJS)
	@$(CC) $(LFLAGS) $(CFLAGS) -o $(ROOTDIR)$(APPNAME) main.c $(COBJS)
	@echo -e "\tLD   $(APPNAME)"

clean:
	
