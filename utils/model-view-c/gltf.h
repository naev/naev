/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "glad.h"

#include "mat4.h"

struct Object_;
typedef struct Object_ Object;

int object_init (void);
void object_exit (void);

Object *object_loadFromFile( const char *filename );
void object_free( Object *obj );

void object_render( const Object *obj, const mat4 *H );

GLuint object_shadowmap( int light );
