/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "colour.h"
#include "opengl.h"

/* Render. */
void background_render( double dt );
void background_renderOverlay( double dt );

/* Add images. */
unsigned int background_addImage( const glTexture *image, double x, double y,
      double move, double scale, double angle, const glColour *col, int foreground );

/* Space Dust. */
void background_initDust( int n );
void background_renderDust( const double dt );
void background_moveDust( double x, double y );

/* Init. */
int background_init (void);
int background_load( const char *name );

/* Clean up. */
void background_clear (void);
void background_free (void);

/* Get textures for any star images and ambient (e.g. nebula) image in the background. */
glTexture** background_getStarTextures (void);
glTexture* background_getAmbientTexture (void);
