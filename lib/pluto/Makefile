LDLIBS= -lm -ldl -llu
# CFLAGS= -g3 -Wall -fprofile-arcs -ftest-coverage
CFLAGS= -g3 -Wall -ansi -pedantic
CFLAGS+= -I../lua

AR= ar rcu
RANLIB= ranlib

LIB= pluto.a
OBJS= pluto.o lzio.o pdep.o


all:
	$(CC) $(CFLAGS) -c -o pluto.o pluto.c
	$(CC) $(CFLAGS) -c -o lzio.o lzio.c
	$(CC) $(CFLAGS) -c -o pdep.o pdep.c
	ar rcu $(LIB) $(OBJS)
	ranlib $(LIB)

clean:
	rm -f $(LIB) $(OBJS)
