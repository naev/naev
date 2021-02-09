/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef OPENGL_RENDER_H
#  define OPENGL_RENDER_H


#include "opengl.h"
#include "opengl_tex.h"
#include "opengl_vbo.h"


/*
 * Init/cleanup.
 */
int gl_initRender (void);
void gl_exitRender (void);


/*
 * Coordinate translation.
 */
void gl_gameToScreenCoords( double *nx, double *ny, double bx, double by );
void gl_screenToGameCoords( double *nx, double *ny, int bx, int by );


/*
 * Rendering.
 */
/* blits texture */
void gl_blitTexture(  const glTexture* texture,
      const double x, const double y,
      const double w, const double h,
      const double tx, const double ty,
      const double tw, const double th, const glColour *c, const double angle );
void gl_blitTextureInterpolate(  const glTexture* ta,
      const glTexture* tb, const double inter,
      const double x, const double y,
      const double w, const double h,
      const double tx, const double ty,
      const double tw, const double th, const glColour *c );
/* blits a sprite, relative pos */
void gl_blitSprite( const glTexture* sprite,
      const double bx, const double by,
      const int sx, const int sy, const glColour *c );
/* Blits a sprite interpolating between textures, relative pos. */
void gl_blitSpriteInterpolate( const glTexture* sa, const glTexture *sb,
      double inter, const double bx, const double by,
      const int sx, const int sy, const glColour *c );
/* Blits a sprite interpolating between textures and scaling, relative pos. */
void gl_blitSpriteInterpolateScale( const glTexture* sa, const glTexture *sb,
      double inter, const double bx, const double by,
      double scalew, double scaleh,
      const int sx, const int sy, const glColour *c );
/* blits a sprite, absolute pos */
void gl_blitStaticSprite( const glTexture* sprite,
      const double bx, const double by,
      const int sx, const int sy, const glColour* c );
/* blits a scaled sprite, absolute pos */
void gl_blitScaleSprite( const glTexture* sprite,
      const double bx, const double by,
      const int sx, const int sy,
      const double bw, const double bh, const glColour* c );
/* blits a texture scaled, absolute pos */
void gl_blitScale( const glTexture* texture,
      const double bx, const double by,
      const double bw, const double bh, const glColour* c );
/* blits the entire image, absolute pos */
void gl_blitStatic( const glTexture* texture,
      const double bx, const double by, const glColour *c );


extern gl_vbo *gl_squareVBO;
void gl_beginSolidProgram(gl_Matrix4 projection, const glColour *c);
void gl_endSolidProgram (void);
void gl_beginSmoothProgram(gl_Matrix4 projection);
void gl_endSmoothProgram (void);


/* Circles. */
void gl_drawCircle( const double x, const double y,
      const double r, const glColour *c, int filled );

/* Lines. */
void gl_drawLine( const double x1, const double y1,
      const double x2, const double y2, const glColour *c );

/* Rectangles. */
void gl_renderRect( double x, double y, double w, double h, const glColour *c );
void gl_renderRectEmpty( double x, double y, double w, double h, const glColour *c );

/* Cross. */
void gl_renderCross( double x, double y, double r, const glColour *c );

/* Triangle. */
void gl_renderTriangleEmpty( double x, double y, double a, double s, double length, const glColour *c );

/* Clipping. */
void gl_clipRect( int x, int y, int w, int h );
void gl_unclipRect (void);


#endif /* OPENGL_RENDER_H */

