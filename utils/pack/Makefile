
ROOTDIR  := ../../
SRCDIR   := $(ROOTDIR)src/
LOCALDIR := utils/pack/

COBJS    := $(SRCDIR)pack.o $(SRCDIR)md5.o main.o

CFLAGS   += -I$(SRCDIR) -O2 -Wall

APPNAME  := pack

.PHONY: all clean

%.o: %.c
	@$(CC) -c $(CFLAGS) -o $@ $<
	@echo -e "\tCC   $(LOCALDIR)$@"

all: $(ROOTDIR)$(APPNAME)

$(ROOTDIR)$(APPNAME): $(COBJS)
	@$(CC) $(CFLAGS) -o $(ROOTDIR)$(APPNAME) $(COBJS)
	@echo -e "\tLD   $(APPNAME)"

clean:
	@$(RM) $(COBJS)
	@echo -e "\tCleaning pack"
