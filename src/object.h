/**
 * Object Loader.
 * http://local.wasp.uwa.edu.au/~pbourke/dataformats/obj/
 */

#ifndef OBJECT_H
#  define OBJECT_H

#include "SDL.h"

#include "opengl_vbo.h"
#include "physics.h"

struct gl_vbo;

typedef struct Material_ {
   char *name;
   GLfloat Ka[3], Kd[3], Ks[3];
   GLfloat Ns, Ni, d;
   GLuint texture;

   unsigned int has_texture:1;
} Material;

typedef struct Mesh_ {
   char *name;
   int num_corners;
   gl_vbo *vbo;
   Material *material;
} Mesh;

typedef struct Object_ {
   Mesh *meshes;
   Material *materials;
} Object;


Object *object_loadFromFile( const char *filename);
void object_render( Object *object );
void object_renderSolid( Object *object, const Solid *solid );
void object_free( Object *object );



#endif
