/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "glad.h"

struct Object_;
typedef struct Object_ Object;

int object_init (void);
void object_exit (void);

Object *object_loadFromFile( const char *filename );
void object_free( Object *obj );

void object_render( const Object *obj, const GLfloat *H );
