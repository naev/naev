
ROOTDIR  := ../../
SRCDIR   := $(ROOTDIR)src/
LOCALDIR := utils/pack/

COBJS    := $(SRCDIR)pack.o $(SRCDIR)md5.o main.o

ifeq ($(OS),WIN32)
CFLAGS   += -D_GNU_SOURCE -W -Wall -I$(ROOTDIR)$(DEPINCDIR)/SDL -D_GNU_SOURCE=1 -Dmain=SDL_main -DGL_GLEXT_PROTOTYPES -I$(ROOTDIR)$(DEPINCDIR)
else
CFLAGS   += -W -Wall $(shell sdl-config --cflags)
endif
ifdef DEBUG
CFLAGS   += -I$(SRCDIR) -g -DDEBUG
ifdef DEBUG_PARANOID
CFLAGS   += -DDEBUG_PARANOID
endif
else
CFLAGS   += -I$(SRCDIR) -O2
endif
ifeq ($(OS),WIN32)
LDFLAGS  += -lmingw32 -mwindows -lSDL -lSDLmain -L$(ROOTDIR)$(DEPBINDIR) -L$(ROOTDIR)$(DEPLIBDIR)
else
LDFLAGS  += $(shell sdl-config --libs)
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
