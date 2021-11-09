CFLAGS=-g3 -W -Wall -Wextra $(shell pkg-config --cflags sdl2 opengl SDL2_image) -I../../src
LIBS=$(shell pkg-config --libs sdl2 opengl SDL2_image) -ldl -lm

main: main.c shader_min.c shader_min.h nstr.c nstr.h common.h gltf.c gltf.h
	$(CC) $< ../../src/glad.c nstr.c shader_min.c gltf.c $(CFLAGS) $(LIBS) -o $@
