/**
 * Object Loader.
 * http://local.wasp.uwa.edu.au/~pbourke/dataformats/obj/
 */

#ifndef OBJECT_H
#  define OBJECT_H

#include <GL/gl.h>

#include "array.h"

typedef struct {
   GLfloat coords[3];
} Vertex;

typedef struct {
   GLushort indices[3];
} Face;

typedef struct {
   Vertex *vertices;
   Face *faces;
} Object;


void object_loadFromFile( const char *filename, Object *object);

#endif
