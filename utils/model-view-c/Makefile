CFLAGS=-g3 -W -Wall -Wextra $(shell pkg-config --cflags sdl2 opengl SDL2_image physfs) -I../../src -I.
LIBS=$(shell pkg-config --libs sdl2 opengl SDL2_image physfs) -ldl -lm

main: main.c ../../src/glad.c nstr.c shader_min.c ../../src/gltf.c ../../src/mat4.c ../../src/vec3.c ../../src/physfsrwops.c
	$(CC) $^ $(CFLAGS) $(LIBS) -o $@
