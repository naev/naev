
ROOTDIR  := ../../
SRCDIR   := $(ROOTDIR)src/
LOCALDIR := utils/pack/

COBJS    := $(SRCDIR)pack.o $(SRCDIR)md5.o main.o

CFLAGS   += -W -Wall $(shell sdl-config --cflags)
LDFLAGS  += $(shell sdl-config --libs)
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
	@$(CC) $(CFLAGS) -o $(ROOTDIR)$(APPNAME) $(COBJS) $(LDFLAGS)
	@echo "   LD   $(APPNAME)"

clean:
	@$(RM) $(COBJS)
	@echo "   Cleaning pack"
