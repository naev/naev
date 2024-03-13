/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "colour.h"

#define OMSG_FONT_DEFAULT_SIZE 16

/*
 * Creation and management.
 */
unsigned int omsg_add( const char *msg, double duration, int fontsize,
                       const glColour *col );
int          omsg_change( unsigned int id, const char *msg, double duration );
int          omsg_exists( unsigned int id );
void         omsg_rm( unsigned int id );

/*
 * Global stuff.
 */
void omsg_position( double center_x, double center_y, double width );
void omsg_cleanup( void );
void omsg_render( double dt );
