
ROOTDIR  := ../../
LOCALDIR := utils/mksprite/

COBJS    := main.o

CFLAGS   := $(shell sdl-config --cflags) $(shell libpng-config --cflags) -O2 -Wall
LDFLAGS  := $(shell libpng-config --libs) $(shell sdl-config --libs) -lSDL_image -lSDL_mixer

APPNAME  := mkspr

.PHONY: all clean

%.o: $(SRCDIR)%.c
	@$(CC) -c $(CFLAGS) -o $@ $<
	@echo "   CC   $(LOCALDIR)$@"

all: $(ROOTDIR)$(APPNAME)

$(ROOTDIR)$(APPNAME): $(COBJS)
	@$(CC) $(LDFLAGS) $(CFLAGS) -o $(ROOTDIR)$(APPNAME) $(COBJS)
	@echo "   LD   $(APPNAME)"

clean:
	@$(RM) $(COBJS)
	@echo "   Cleaning mkspr"
	
