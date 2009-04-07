
ROOTDIR  := ../../
SRCDIR   := $(ROOTDIR)src/
LOCALDIR := utils/pack/

COBJS    := $(SRCDIR)pack.o $(SRCDIR)md5.o main.o

ifeq ($(OS),WIN32)
CFLAGS   += -D_GNU_SOURCE -W -Wall
CFLAGS   += -I$(ROOTDIR)$(DEP_SDL_INC_DIR) -D_GNU_SOURCE=1 -Dmain=SDL_main -DGL_GLEXT_PROTOTYPES
CFLAGS   += -I$(ROOTDIR)$(DEP_INC_DIR)
LDFLAGS  += -lmingw32 -mwindows
LDFLAGS  += -lSDL -lSDLmain
LDFLAGS  += -L$(ROOTDIR)$(DEP_BIN_DIR) -L$(ROOTDIR)$(DEP_LIB_DIR)
else
CFLAGS   += -W -Wall $(shell sdl-config --cflags)
LDFLAGS  += $(shell sdl-config --libs)
endif
ifdef DEBUG
CFLAGS   += -I$(SRCDIR) -g -DDEBUG
ifdef DEBUG_PARANOID
CFLAGS   += -DDEBUG_PARANOID
endif
else
CFLAGS   += -I$(SRCDIR) -O2
endif

APPNAME  := pack

.PHONY: all clean

%.o: %.c
	@$(CC) -c $(CFLAGS) -o $@ $<
	@echo "   CC   $(LOCALDIR)$@"

all: $(ROOTDIR)$(APPNAME)

$(ROOTDIR)$(APPNAME): $(COBJS)
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $(ROOTDIR)$(APPNAME) $(COBJS)
	@echo "   LD   $(APPNAME)"

clean:
	@$(RM) $(COBJS)
	@echo "   Cleaning pack"
