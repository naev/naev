/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "glad.h"

#include "mat4.h"

struct Object_;
typedef struct Object_ Object;

/* Framework itself. */
int object_init (void);
void object_exit (void);

/* Loading and freeing. */
Object *object_loadFromFile( const char *filename );
void object_free( Object *obj );

/* Rendering and updating. */
void object_update( Object *obj, double dt );
void object_render( const Object *obj, const mat4 *H );

/* Lighting. */
void object_lightAmbient( double r, double g, double b );

/* Misc functions. */
GLuint object_shadowmap( int light );
