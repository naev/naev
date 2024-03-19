/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "mat4.h"
#include "opengl.h"
#include "opengl_tex.h"
#include "opengl_vbo.h"
#include "shaders.gen.h"

/*
 * Init/cleanup.
 */
int  gl_initRender( void );
void gl_exitRender( void );

/*
 * Coordinate translation.
 */
void gl_gameToScreenCoords( double *nx, double *ny, double bx, double by );
__attribute__( ( const ) ) mat4 gl_gameToScreenMatrix( mat4 lhs );
void gl_screenToGameCoords( double *nx, double *ny, int bx, int by );

/*
 * Rendering.
 */
/* blits texture */
void gl_renderTextureRawH( GLuint texture, const mat4 *projection,
                           const mat4 *tex_mat, const glColour *c );
void gl_renderTextureRaw( GLuint texture, uint8_t flags, double x, double y,
                          double w, double h, double tx, double ty, double tw,
                          double th, const glColour *c, double angle );
void gl_renderTexture( const glTexture *texture, double x, double y, double w,
                       double h, double tx, double ty, double tw, double th,
                       const glColour *c, double angle );
void gl_renderTextureInterpolateRawH( GLuint ta, GLuint tb, double inter,
                                      const mat4 *projection,
                                      const mat4 *tex_mat, const glColour *c );
void gl_renderTextureInterpolate( const glTexture *ta, const glTexture *tb,
                                  double inter, double x, double y, double w,
                                  double h, double tx, double ty, double tw,
                                  double th, const glColour *c );
/* blits a sprite, relative pos */
void gl_renderSprite( const glTexture *sprite, double bx, double by, int sx,
                      int sy, const glColour *c );
void gl_renderSpriteScale( const glTexture *sprite, double bx, double by,
                           double scalew, double scaleh, int sx, int sy,
                           const glColour *c );
void gl_renderSpriteRotate( const glTexture *sprite, double bx, double by,
                            double angle, int sx, int sy, const glColour *c );
void gl_renderSpriteScaleRotate( const glTexture *sprite, double bx, double by,
                                 double scalew, double scaleh, double angle,
                                 int sx, int sy, const glColour *c );
/* Blits a sprite interpolating between textures, relative pos. */
void gl_renderSpriteInterpolate( const glTexture *sa, const glTexture *sb,
                                 double inter, double bx, double by, int sx,
                                 int sy, const glColour *c );
void gl_renderStaticSpriteInterpolate( const glTexture *sa, const glTexture *sb,
                                       double inter, double bx, double by,
                                       int sx, int sy, const glColour *c );
void gl_renderStaticSpriteInterpolateScale( const glTexture *sa,
                                            const glTexture *sb, double inter,
                                            double bx, double by, double scalew,
                                            double scaleh, int sx, int sy,
                                            const glColour *c );
/* Blits a sprite interpolating between textures and scaling, relative pos. */
void gl_renderSpriteInterpolateScale( const glTexture *sa, const glTexture *sb,
                                      double inter, double bx, double by,
                                      double scalew, double scaleh, int sx,
                                      int sy, const glColour *c );
/* blits a sprite, absolute pos */
void gl_renderStaticSprite( const glTexture *sprite, double bx, double by,
                            int sx, int sy, const glColour *c );
/* blits a scaled sprite, absolute pos */
void gl_renderScaleSprite( const glTexture *sprite, double bx, double by,
                           int sx, int sy, double bw, double bh,
                           const glColour *c );
/* blits a texture scaled, absolute pos */
void gl_renderScale( const glTexture *texture, double bx, double by, double bw,
                     double bh, const glColour *c );
/* blits a texture scaled to a rectangle, but conserve aspect ratio, absolute
 * pos */
void gl_renderScaleAspect( const glTexture *texture, double bx, double by,
                           double bw, double bh, const glColour *c );
/* blits the entire image, absolute pos */
void gl_renderStatic( const glTexture *texture, double bx, double by,
                      const glColour *c );
void gl_renderSDF( const glTexture *texture, double x, double y, double w,
                   double h, const glColour *c, double angle, double outline );

extern gl_vbo *gl_squareVBO;
extern gl_vbo *gl_paneVBO;
extern gl_vbo *gl_circleVBO;

void gl_beginSolidProgram( mat4 projection, const glColour *c );
void gl_endSolidProgram( void );
void gl_beginSmoothProgram( mat4 projection );
void gl_endSmoothProgram( void );

/* Simple Shaders. */
void gl_renderShader( double x, double y, double w, double h, double r,
                      const SimpleShader *shd, const glColour *c, int center );
void gl_renderShaderH( const SimpleShader *shd, const mat4 *H,
                       const glColour *c, int center );

/* Circles. */
#define gl_renderDisk( x, y, r, c )                                            \
   gl_renderCircle( ( x ), ( y ), ( r ), ( c ), 0 )
void gl_renderCircle( double x, double y, double r, const glColour *c,
                      double line_width );
void gl_renderCircleH( const mat4 *H, const glColour *c, double line_width );

/* Lines. */
void gl_renderLine( double x1, double y1, double x2, double y2,
                    const glColour *c );

/* Rectangles. */
#define gl_renderPane( x, y, w, h, c )                                         \
   gl_renderRoundPane( ( x ), ( y ), ( w ), ( h ), 0, 0, ( c ) )
#define gl_renderRect( x, y, w, h, lw, c )                                     \
   gl_renderRoundRect( ( x ), ( y ), ( w ), ( h ), ( lw ), 0, 0, ( c ) )
void gl_renderRoundPane( double x, double y, double w, double h, double rx,
                         double ry, const glColour *c );
void gl_renderRoundRect( double x, double y, double w, double h,
                         double line_width, double rx, double ry,
                         const glColour *c );
void gl_renderRectH( const mat4 *H, const glColour *c, int line_width, int rx,
                     int ry );

/* Cross. */
void gl_renderCross( double x, double y, double r, const glColour *c );

/* Triangle. */
void gl_renderTriangleEmpty( double x, double y, double a, double s,
                             double length, const glColour *c );

/* Clipping. */
void gl_clipRect( int x, int y, int w, int h );
void gl_unclipRect( void );
