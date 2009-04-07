
ROOTDIR  := ../../
LOCALDIR := utils/mksprite/

COBJS    := main.o

ifeq ($(OS),WIN32)
CFLAGS   := -D_GNU_SOURCE -O2 -Wall
CFLAGS   += -I$(ROOTDIR)$(DEP_SDL_INC_DIR) -D_GNU_SOURCE=1 -Dmain=SDL_main -DGL_GLEXT_PROTOTYPES
CFLAGS   += -I$(ROOTDIR)$(DEP_PNG_INC_DIR)
LDFLAGS  := -lmingw32 -mwindows
LDFLAGS  += -lSDL -lSDLmain -lSDL_image -lSDL_mixer
LDFLAGS  += -lpng12
LDFLAGS  += -L$(ROOTDIR)$(DEP_BIN_DIR) -L$(ROOTDIR)$(DEP_LIB_DIR)
else
CFLAGS   := $(shell sdl-config --cflags) $(shell libpng-config --cflags) -O2 -Wall
LDFLAGS  := $(shell libpng-config --libs) $(shell sdl-config --libs) -lSDL_image -lSDL_mixer
endif

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
	
