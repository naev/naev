
ROOTDIR = ../../
SRCDIR = $(ROOTDIR)src/
LOCALDIR = utils/pack/

COBJS = pack.o md5.o

CFLAGS += -I$(SRCDIR) -O2 -Wall

APPNAME = pack

%.o: $(SRCDIR)%.c
	@$(CC) -c $(CFLAGS) -o $@ $<
	@echo -e "\tCC   $(LOCALDIR)$@"

all: $(COBJS)
	@$(CC) $(CFLAGS) -o $(ROOTDIR)$(APPNAME) main.c $(COBJS)
	@echo -e "\tLD   $(APPNAME)"

clean:
	@rm -fr *.o
