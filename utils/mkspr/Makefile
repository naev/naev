
ROOTDIR  := ../../
LOCALDIR := utils/mksprite/

COBJS    := main.o

CFLAGS   := $(shell sdl-config --cflags) $(shell libpng-config --cflags) -O2 -Wall
LDFLAGS  := $(shell libpng-config --libs) $(shell sdl-config --libs) -lSDL_image

APPNAME  := mkspr

.PHONY: all clean

%.o: $(SRCDIR)%.c
	@$(CC) -c $(CFLAGS) -o $@ $<
	@echo "   CC   $(LOCALDIR)$@"

all: $(ROOTDIR)$(APPNAME)

$(ROOTDIR)$(APPNAME): $(COBJS)
ifeq ($(OS),WIN32)
	$(CC) -lmingw32 -mwindows $(CFLAGS) -o $(ROOTDIR)$(APPNAME) $(COBJS) $(LDFLAGS)
else
	@$(CC) $(LDFLAGS) $(CFLAGS) -o $(ROOTDIR)$(APPNAME) $(COBJS)
endif
	@echo "   LD   $(APPNAME)"

clean:
	@$(RM) $(COBJS)
	@echo "   Cleaning mkspr"
	
