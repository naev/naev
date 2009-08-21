/**
 * Object Loader.
 * http://local.wasp.uwa.edu.au/~pbourke/dataformats/obj/
 */

#ifndef OBJECT_H
#  define OBJECT_H

#include <GL/gl.h>

#include "array.h"


typedef struct {
   int num_corners;
   GLuint object;
   GLuint texture;
} Object;


Object *object_loadFromFile( const char *filename);
void object_render( Object *object );



#endif
