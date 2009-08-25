/**
 * Object Loader.
 * http://local.wasp.uwa.edu.au/~pbourke/dataformats/obj/
 */

#ifndef OBJECT_H
#  define OBJECT_H

#include <GL/gl.h>

#include "opengl_vbo.h"

struct gl_vbo;

typedef struct {
   int num_corners;
   gl_vbo *object;
   GLuint texture;
} Object;


Object *object_loadFromFile( const char *filename);
void object_render( Object *object );



#endif
