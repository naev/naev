CFLAGS=-g3 -W -Wall -Wextra $(shell pkg-config --cflags sdl2 opengl) -I../../src
LIBS=$(shell pkg-config --libs sdl2 opengl) -ldl -lm -lSDL_image

main: main.c shader_min.c shader_min.h nstr.c nstr.h common.h
	$(CC) $< ../../src/glad.c nstr.c shader_min.c $(CFLAGS) $(LIBS) -o $@
