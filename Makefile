
#DEBUG = 1

APPNAME = main

OBJS := main.o		\
		log.o			\
		physics.o

CFLAGS = -Wall
ifdef DEBUG
CFLAGS += -g3
else # DEBUG
CFLAGS += -O2
endif # DEBUG

LDFLAGS = -lm


%.o:	%.c
	gcc -c $(CFLAGS) -o $@ $<


all:	$(OBJS)
	gcc $(LDFLAGS) -o $(APPNAME) $(OBJS)


clean:
	rm -f $(OBJS)
