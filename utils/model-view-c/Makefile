CFLAGS=-g3 -W -Wall -Wextra $(shell pkg-config --cflags sdl2 opengl)
LIBS=$(shell pkg-config --libs sdl2 opengl) -ldl -lSDL_image

main: main.c
	$(CC) $< ../../src/glad.c $(CFLAGS) $(LIBS) -o $@
