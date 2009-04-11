/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef OPENGL_RENDER_H
#  define OPENGL_RENDER_H


#include "opengl.h"


/*
 * Init/cleanup.
 */
int gl_initRender (void);
void gl_exitRender (void);


/*
 * Camera.
 */
void gl_cameraBind( Vector2d* pos );
void gl_cameraStatic( double x, double y );
void gl_cameraGet( double *x, double *y );


/*
 * Rendering.
 */
/* blits a sprite, relative pos */
void gl_blitSprite( const glTexture* sprite,
      const double bx, const double by,
      const int sx, const int sy, const glColour *c );
/* blits a sprite, absolute pos */
void gl_blitStaticSprite( const glTexture* sprite,
      const double bx, const double by,
      const int sx, const int sy, const glColour* c );
/* blits a texture scaled, absolute pos */
void gl_blitScale( const glTexture* texture,
      const double bx, const double by,
      const double bw, const double bh, const glColour* c );
/* blits the entire image, absolute pos */
void gl_blitStatic( const glTexture* texture,
      const double bx, const double by, const glColour *c );
/* circle drawing */
void gl_drawCircle( const double x, const double y, const double r );
void gl_drawCircleInRect( const double x, const double y, const double r,
      const double rx, const double ry, const double rw, const double rh );


#endif /* OPENGL_RENDER_H */
   
