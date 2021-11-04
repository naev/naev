/**
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include "SDL.h"
/** @endcond */

#include "opengl.h"
#include "physics.h"

struct Object_;
typedef struct Object_ Object;

Object *object_loadFromFile( const char *filename );
void object_renderSolidPart( const Object *object, const Solid *solid, const char *part_name, GLfloat alpha, double scale );
void object_free( Object *object );
