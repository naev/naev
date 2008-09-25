
ROOTDIR  := ../../
LOCALDIR := utils/mksprite/

COBJS    := main.o

CFLAGS   := $(shell sdl-config --cflags) -O2 -Wall
LFLAGS   := $(sdl-config --libs) -lSDL_image

APPNAME  := mkspr

.PHONY: all clean

%.o: $(SRCDIR)%.c
	@$(CC) -c $(CFLAGS) -o $@ $<
	@echo -e "\tCC   $(LOCALDIR)$@"

all: $(ROOTDIR)$(APPNAME)

$(ROOTDIR)$(APPNAME): $(COBJS)
	@$(CC) $(LFLAGS) $(CFLAGS) -o $(ROOTDIR)$(APPNAME) $(COBJS)
	@echo -e "\tLD   $(APPNAME)"

clean:
	@$(RM) $(COBJS)
	@echo -e "\tCleaning mkspr"
	
