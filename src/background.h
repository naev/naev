/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef BACKGROUND_H
#  define BACKGROUND_H


#include "opengl.h"
#include "colour.h"


/* Render. */
void background_render( double dt );

/* Add images. */
int background_addImage( glTexture *image, double x, double y,
      double xmove, double ymove, double scale, glColour *col );

/* Stars. */
void background_initStars( int n );
void background_renderStars( const double dt );


/* Clean up. */
void background_clear (void);
void background_free (void);


#endif /* BACKGROUND_H */


