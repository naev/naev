
ROOTDIR  := ../../
LOCALDIR := utils/mksprite/

COBJS    := main.o

CFLAGS   := $(shell sdl-config --cflags) $(shell libpng-config --cflags) -O2 -Wall
LFLAGS   := $(sdl-config --libs) -lSDL_image $(shell libpng-config --libs)

APPNAME  := mkspr

.PHONY: all clean

%.o: $(SRCDIR)%.c
	@$(CC) -c $(CFLAGS) -o $@ $<
	@echo "   CC   $(LOCALDIR)$@"

all: $(ROOTDIR)$(APPNAME)

$(ROOTDIR)$(APPNAME): $(COBJS)
	@$(CC) $(LFLAGS) $(CFLAGS) -o $(ROOTDIR)$(APPNAME) $(COBJS)
	@echo "   LD   $(APPNAME)"

clean:
	@$(RM) $(COBJS)
	@echo "   Cleaning mkspr"
	
