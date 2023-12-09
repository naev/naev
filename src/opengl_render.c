/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file opengl_render.c
 *
 * @brief This file handles the opengl rendering routines.
 *
 * There are two coordinate systems: relative and absolute.
 *
 * Relative:
 *  * Everything is drawn relative to the player, if it doesn't fit on screen
 *    it is clipped.
 *  * Origin (0., 0.) would be ontop of the player.
 *
 * Absolute:
 *  * Everything is drawn in "screen coordinates".
 *  * (0., 0.) is bottom left.
 *  * (SCREEN_W, SCREEN_H) is top right.
 *
 * Note that the game actually uses a third type of coordinates for when using
 *  raw commands.  In this third type, the (0.,0.) is actually in middle of the
 *  screen.  (-SCREEN_W/2.,-SCREEN_H/2.) is bottom left and
 *  (+SCREEN_W/2.,+SCREEN_H/2.) is top right.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "opengl_render.h"

#include "camera.h"
#include "conf.h"
#include "gui.h"
#include "log.h"
#include "ndata.h"
#include "nstring.h"
#include "opengl.h"

#define OPENGL_RENDER_VBO_SIZE      256 /**< Size of VBO. */

static gl_vbo *gl_renderVBO = 0; /**< VBO for rendering stuff. */
gl_vbo *gl_squareVBO = 0;
static gl_vbo *gl_squareEmptyVBO = 0;
gl_vbo *gl_circleVBO = 0;
static gl_vbo *gl_lineVBO = 0;
static gl_vbo *gl_triangleVBO = 0;
static int gl_renderVBOtexOffset = 0; /**< VBO texture offset. */
static int gl_renderVBOcolOffset = 0; /**< VBO colour offset. */

void gl_beginSolidProgram(mat4 projection, const glColour *c)
{
   glUseProgram(shaders.solid.program);
   glEnableVertexAttribArray(shaders.solid.vertex);
   gl_uniformColour(shaders.solid.colour, c);
   gl_uniformMat4(shaders.solid.projection, &projection);
}

void gl_endSolidProgram (void)
{
   glDisableVertexAttribArray(shaders.solid.vertex);
   glUseProgram(0);
   gl_checkErr();
}

void gl_beginSmoothProgram(mat4 projection)
{
   glUseProgram(shaders.smooth.program);
   glEnableVertexAttribArray(shaders.smooth.vertex);
   glEnableVertexAttribArray(shaders.smooth.vertex_colour);
   gl_uniformMat4(shaders.smooth.projection, &projection);
}

void gl_endSmoothProgram() {
   glDisableVertexAttribArray(shaders.smooth.vertex);
   glDisableVertexAttribArray(shaders.smooth.vertex_colour);
   glUseProgram(0);
   gl_checkErr();
}

/**
 * @brief Renders a rectangle.
 *
 *    @param x X position to render rectangle at.
 *    @param y Y position to render rectangle at.
 *    @param w Rectangle width.
 *    @param h Rectangle height.
 *    @param c Rectangle colour.
 */
void gl_renderRect( double x, double y, double w, double h, const glColour *c )
{
   /* Set the vertex. */
   mat4 projection = gl_view_matrix;
   mat4_translate( &projection, x, y, 0. );
   mat4_scale( &projection, w, h, 1. );

   gl_renderRectH( &projection, c, 1 );
}

/**
 * @brief Renders a rectangle.
 *
 *    @param x X position to render rectangle at.
 *    @param y Y position to render rectangle at.
 *    @param w Rectangle width.
 *    @param h Rectangle height.
 *    @param c Rectangle colour.
 */
void gl_renderRectEmpty( double x, double y, double w, double h, const glColour *c )
{
   mat4 projection = gl_view_matrix;
   mat4_translate( &projection, x, y, 0. );
   mat4_scale( &projection, w, h, 1. );

   gl_renderRectH( &projection, c, 0 );
}

/**
 * @brief Renders a rectangle.
 *
 *    @param H Transformation matrix to apply.
 *    @param filled Whether or not to fill.
 *    @param c Rectangle colour.
 */
void gl_renderRectH( const mat4 *H, const glColour *c, int filled )
{
   gl_beginSolidProgram(*H, c);
   if (filled) {
      gl_vboActivateAttribOffset( gl_squareVBO, shaders.solid.vertex, 0, 2, GL_FLOAT, 0 );
      glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
   }
   else {
      gl_vboActivateAttribOffset( gl_squareEmptyVBO, shaders.solid.vertex, 0, 2, GL_FLOAT, 0 );
      glDrawArrays( GL_LINE_STRIP, 0, 5 );
   }
   gl_endSolidProgram();
}

/**
 * @brief Renders a cross at a given position.
 *
 *    @param x X position to center at.
 *    @param y Y position to center at.
 *    @param r Radius of cross.
 *    @param c Colour to use.
 */
void gl_renderCross( double x, double y, double r, const glColour *c )
{
   glUseProgram(shaders.crosshairs.program);
   glUniform1f(shaders.crosshairs.paramf, 1.); /* No outline. */
   gl_renderShader( x, y, r, r, 0., &shaders.crosshairs, c, 1 );
}

/**
 * @brief Renders a triangle at a given position.
 *
 *    @param x X position to center at.
 *    @param y Y position to center at.
 *    @param a Angle the triangle should "face" (right is 0.)
 *    @param s Scaling of the triangle.
 *    @param length Length deforming factor. Setting it to a value of other than 1. moves away from an equilateral triangle.
 *    @param c Colour to use.
 */
void gl_renderTriangleEmpty( double x, double y, double a, double s, double length, const glColour *c )
{
   mat4 projection = gl_view_matrix;
   mat4_translate( &projection, x, y, 0. );
   if (a != 0.)
      mat4_rotate2d( &projection, a );
   mat4_scale( &projection, s*length, s, 1. );

   gl_beginSolidProgram(projection, c);
   gl_vboActivateAttribOffset( gl_triangleVBO, shaders.solid.vertex, 0, 2, GL_FLOAT, 0 );
   glDrawArrays( GL_LINE_STRIP, 0, 4 );
   gl_endSolidProgram();
}

/**
 * @brief Texture blitting backend.
 *
 *    @param texture Texture to blit.
 *    @param flags Texture flags,.
 *    @param x X position of the texture on the screen. (units pixels)
 *    @param y Y position of the texture on the screen. (units pixels)
 *    @param w Width on the screen. (units pixels)
 *    @param h Height on the screen. (units pixels)
 *    @param tx X position within the texture. [0:1]
 *    @param ty Y position within the texture. [0:1]
 *    @param tw Texture width. [0:1]
 *    @param th Texture height. [0:1]
 *    @param c Colour to use (modifies texture colour).
 *    @param angle Rotation to apply (radians ccw around the center).
 */
void gl_renderTextureRaw( GLuint texture, uint8_t flags,
      double x, double y, double w, double h,
      double tx, double ty, double tw, double th,
      const glColour *c, double angle )
{
   // Half width and height
   double hw, hh;
   mat4 projection, tex_mat;

   glUseProgram(shaders.texture.program);

   /* Bind the texture. */
   glBindTexture( GL_TEXTURE_2D, texture);

   /* Must have colour for now. */
   if (c == NULL)
      c = &cWhite;

   hw = w*0.5;
   hh = h*0.5;

   /* Set the vertex. */
   projection = gl_view_matrix;
   if (angle==0.) {
     mat4_translate( &projection, x, y, 0. );
     mat4_scale( &projection, w, h, 1. );
   }
   else {
     mat4_translate( &projection, x+hw, y+hh, 0. );
     mat4_rotate2d( &projection, angle );
     mat4_translate( &projection, -hw, -hh, 0. );
     mat4_scale( &projection, w, h, 1. );
   }
   glEnableVertexAttribArray( shaders.texture.vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.texture.vertex,
         0, 2, GL_FLOAT, 0 );

   /* Set the texture. */
   tex_mat = (flags & OPENGL_TEX_VFLIP) ? mat4_ortho(-1, 1, 2, 0, 1, -1) : mat4_identity();
   mat4_translate( &tex_mat, tx, ty, 0. );
   mat4_scale( &tex_mat, tw, th, 1. );

   /* Set shader uniforms. */
   gl_uniformColour(shaders.texture.colour, c);
   gl_uniformMat4(shaders.texture.projection, &projection);
   gl_uniformMat4(shaders.texture.tex_mat, &tex_mat);

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shaders.texture.vertex );

   /* anything failed? */
   gl_checkErr();

   glUseProgram(0);
}

/**
 * @brief Texture blitting backend.
 *
 *    @param texture Texture to blit.
 *    @param x X position of the texture on the screen. (units pixels)
 *    @param y Y position of the texture on the screen. (units pixels)
 *    @param w Width on the screen. (units pixels)
 *    @param h Height on the screen. (units pixels)
 *    @param tx X position within the texture. [0:1]
 *    @param ty Y position within the texture. [0:1]
 *    @param tw Texture width. [0:1]
 *    @param th Texture height. [0:1]
 *    @param c Colour to use (modifies texture colour).
 *    @param angle Rotation to apply (radians ccw around the center).
 */
void gl_renderTexture( const glTexture* texture,
      double x, double y, double w, double h,
      double tx, double ty, double tw, double th,
      const glColour *c, double angle )
{
   gl_renderTextureRaw( texture->texture, texture->flags, x, y, w, h, tx, ty, tw, th, c, angle );
}

/**
 * @brief SDF Texture blitting backend.
 *
 *    @param texture Texture to blit.
 *    @param x X position of the texture on the screen. (units pixels)
 *    @param y Y position of the texture on the screen. (units pixels)
 *    @param w Width on the screen. (units pixels)
 *    @param h Height on the screen. (units pixels)
 *    @param c Colour to use (modifies texture colour).
 *    @param angle Rotation to apply (radians ccw around the center).
 *    @param outline Thickness of the outline.
 */
void gl_renderSDF( const glTexture *texture,
      double x, double y, double w, double h,
      const glColour *c, double angle, double outline )
{
   (void) outline; /* TODO handle outline. */
   double hw, hh; /* Half width and height */
   double sw, sh;
   mat4 projection, tex_mat;

   glUseProgram(shaders.texturesdf.program);

   /* Bind the texture. */
   glBindTexture( GL_TEXTURE_2D, texture->texture );

   /* Must have colour for now. */
   if (c == NULL)
      c = &cWhite;

   hw = w*0.5;
   hh = h*0.5;

   /* Set the vertex. */
   projection = gl_view_matrix;
   if (angle==0.) {
     mat4_translate( &projection, x+hw, y+hh, 0. );
     mat4_scale( &projection, hw, hh, 1. );
   }
   else {
     mat4_translate( &projection, x+hw, y+hh, 0. );
     mat4_rotate2d( &projection, angle );
     mat4_scale( &projection, hw, hh, 1. );
   }
   glEnableVertexAttribArray( shaders.texturesdf.vertex );
   gl_vboActivateAttribOffset( gl_circleVBO, shaders.texturesdf.vertex,
         0, 2, GL_FLOAT, 0 );

   /* Set the texture. */
   /* TODO we would want to pad the texture a bit to get nice marked borders, but we have to actually pad the SDF first... */
   sw = 0.;//1./w;
   sh = 0.;//1./h;
   tex_mat = (texture->flags & OPENGL_TEX_VFLIP) ? mat4_ortho(-1, 1, 2, 0, 1, -1) : mat4_identity();
   mat4_scale( &tex_mat, texture->srw+2.*sw, texture->srh+2.*sh, 1. );
   mat4_translate( &tex_mat, -sw, -sh, 0. );

   /* Set shader uniforms. */
   gl_uniformColour(shaders.texturesdf.colour, c);
   gl_uniformMat4(shaders.texturesdf.projection, &projection);
   gl_uniformMat4(shaders.texturesdf.tex_mat, &tex_mat);
   glUniform1f( shaders.texturesdf.m, (2.0*texture->vmax*(w+2.)/texture->w) );

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shaders.texturesdf.vertex );

   /* anything failed? */
   gl_checkErr();

   glUseProgram(0);
}

/**
 * @brief Texture blitting backend for interpolated texture.
 *
 * Value blitted is  ta*inter + tb*(1.-inter).
 *
 *    @param ta Texture A to blit.
 *    @param tb Texture B to blit.
 *    @param inter Amount of interpolation to do.
 *    @param x X position of the texture on the screen.
 *    @param y Y position of the texture on the screen.
 *    @param w Width on the screen. (units pixels)
 *    @param h Height on the screen. (units pixels)
 *    @param tx X position within the texture.
 *    @param ty Y position within the texture.
 *    @param tw Texture width.
 *    @param th Texture height.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderTextureInterpolate(  const glTexture* ta,
      const glTexture* tb, double inter,
      double x, double y, double w, double h,
      double tx, double ty, double tw, double th, const glColour *c )
{
   /* No interpolation. */
   if (tb == NULL) {
      gl_renderTexture( ta, x, y, w, h, tx, ty, tw, th, c, 0. );
      return;
   }

   /* Corner cases. */
   if (inter >= 1.) {
      gl_renderTexture( ta, x, y, w, h, tx, ty, tw, th, c, 0. );
      return;
   }
   else if (inter <= 0.) {
      gl_renderTexture( tb, x, y, w, h, tx, ty, tw, th, c, 0. );
      return;
   }

   mat4 projection, tex_mat;

   glUseProgram(shaders.texture_interpolate.program);

   /* Bind the textures. */
   glActiveTexture( GL_TEXTURE1 );
   glBindTexture( GL_TEXTURE_2D, tb->texture);
   glActiveTexture( GL_TEXTURE0 );
   glBindTexture( GL_TEXTURE_2D, ta->texture);
   /* Always end with TEXTURE0 active. */

   /* Must have colour for now. */
   if (c == NULL)
      c = &cWhite;

   /* Set the vertex. */
   projection = gl_view_matrix;
   mat4_translate( &projection, x, y, 0. );
   mat4_scale( &projection, w, h, 1. );
   glEnableVertexAttribArray( shaders.texture_interpolate.vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.texture_interpolate.vertex, 0, 2, GL_FLOAT, 0 );

   /* Set the texture. */
   tex_mat = (ta->flags & OPENGL_TEX_VFLIP) ? mat4_ortho(-1, 1, 2, 0, 1, -1) : mat4_identity();
   mat4_translate( &tex_mat, tx, ty, 0. );
   mat4_scale( &tex_mat, tw, th, 1. );

   /* Set shader uniforms. */
   glUniform1i(shaders.texture_interpolate.sampler1, 0);
   glUniform1i(shaders.texture_interpolate.sampler2, 1);
   gl_uniformColour(shaders.texture_interpolate.colour, c);
   glUniform1f(shaders.texture_interpolate.inter, inter);
   gl_uniformMat4(shaders.texture_interpolate.projection, &projection);
   gl_uniformMat4(shaders.texture_interpolate.tex_mat, &tex_mat);

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shaders.texture_interpolate.vertex );

   /* anything failed? */
   gl_checkErr();

   glUseProgram(0);
}

/**
 * @brief Converts in-game coordinates to screen coordinates.
 *
 *    @param[out] nx New screen X coord.
 *    @param[out] ny New screen Y coord.
 *    @param bx Game X coord to translate.
 *    @param by Game Y coord to translate.
 */
void gl_gameToScreenCoords( double *nx, double *ny, double bx, double by )
{
   double cx,cy, gx,gy, z;

   /* Get parameters. */
   cam_getPos( &cx, &cy );
   z = cam_getZoom();
   gui_getOffset( &gx, &gy );

   /* calculate position - we'll use relative coords to player */
   *nx = (bx - cx) * z + gx + SCREEN_W*0.5;
   *ny = (by - cy) * z + gy + SCREEN_H*0.5;
}

/**
 * @brief Return a transformation which converts in-game coordinates to screen coordinates.
 *
 *    @param lhs Matrix to multiply by the conversion matrix.
 */
mat4 gl_gameToScreenMatrix( mat4 lhs )
{
   double cx,cy, gx,gy, z;
   mat4 projection = lhs;

   /* Get parameters. */
   cam_getPos( &cx, &cy );
   z = cam_getZoom();
   gui_getOffset( &gx, &gy );

   mat4_translate( &projection, gx + SCREEN_W*0.5, gy + SCREEN_H*0.5, 0. );
   mat4_scale( &projection, z, z, 1. );
   mat4_translate( &projection, -cx, cy, 0. );
   return projection;
}

/**
 * @brief Converts screen coordinates to in-game coordinates.
 *
 *    @param[out] nx New in-game X coord.
 *    @param[out] ny New in-game Y coord.
 *    @param bx Screen X coord to translate.
 *    @param by Screen Y coord to translate.
 */
void gl_screenToGameCoords( double *nx, double *ny, int bx, int by )
{
   double cx,cy, gx,gy, z;

   /* Get parameters. */
   cam_getPos( &cx, &cy );
   z = cam_getZoom();
   gui_getOffset( &gx, &gy );

   /* calculate position - we'll use relative coords to player */
   *nx = (bx - SCREEN_W*0.5 - gx) / z + cx;
   *ny = (by - SCREEN_H*0.5 - gy) / z + cy;
}

/**
 * @brief Blits a sprite, position is relative to the player.
 *
 * Since position is in "game coordinates" it is subject to all
 * sorts of position transformations.
 *
 *    @param sprite Sprite to blit.
 *    @param bx X position of the texture relative to the player.
 *    @param by Y position of the texture relative to the player.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderSprite( const glTexture* sprite, double bx, double by,
      int sx, int sy, const glColour* c )
{
   double x,y, w,h, tx,ty, z;

   /* Translate coords. */
   z = cam_getZoom();
   gl_gameToScreenCoords( &x, &y, bx - sprite->sw*0.5, by - sprite->sh*0.5 );

   /* Scaled sprite dimensions. */
   w = sprite->sw*z;
   h = sprite->sh*z;

   /* check if inbounds */
   if ((x < -w) || (x > SCREEN_W+w) ||
         (y < -h) || (y > SCREEN_H+h))
      return;

   /* texture coords */
   tx = sprite->sw*(double)(sx)/sprite->w;
   ty = sprite->sh*(sprite->sy-(double)sy-1)/sprite->h;

   gl_renderTexture( sprite, x, y, w, h,
         tx, ty, sprite->srw, sprite->srh, c, 0. );
}

/**
 * @brief Blits a sprite, position is relative to the player.
 *
 * Since position is in "game coordinates" it is subject to all
 * sorts of position transformations.
 *
 *    @param sprite Sprite to blit.
 *    @param bx X position of the texture relative to the player.
 *    @param by Y position of the texture relative to the player.
 *    @param scalew Scaling of width.
 *    @param scaleh Scaling of height.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderSpriteScale( const glTexture* sprite, double bx, double by,
      double scalew, double scaleh,
      int sx, int sy, const glColour* c )
{
   double x,y, w,h, tx,ty, z;

   /* Translate coords. */
   z = cam_getZoom();
   gl_gameToScreenCoords( &x, &y, bx - sprite->sw*0.5, by - sprite->sh*0.5 );

   /* Scaled sprite dimensions. */
   w = sprite->sw*z*scalew;
   h = sprite->sh*z*scaleh;

   /* check if inbounds */
   if ((x < -w) || (x > SCREEN_W+w) ||
         (y < -h) || (y > SCREEN_H+h))
      return;

   /* texture coords */
   tx = sprite->sw*(double)(sx)/sprite->w;
   ty = sprite->sh*(sprite->sy-(double)sy-1)/sprite->h;

   gl_renderTexture( sprite, x, y, w, h,
         tx, ty, sprite->srw, sprite->srh, c, 0. );
}

/**
 * @brief Blits a sprite, position is relative to the player with rotation.
 *
 * Since position is in "game coordinates" it is subject to all
 * sorts of position transformations.
 *
 *    @param sprite Sprite to blit.
 *    @param bx X position of the texture relative to the player.
 *    @param by Y position of the texture relative to the player.
 *    @param angle Angle to rotate when rendering.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderSpriteRotate( const glTexture* sprite,
      double bx, double by, double angle,
      int sx, int sy, const glColour *c )
{
   double x,y, w,h, tx,ty, z;

   /* Translate coords. */
   z = cam_getZoom();
   gl_gameToScreenCoords( &x, &y, bx - sprite->sw*0.5, by - sprite->sh*0.5 );

   /* Scaled sprite dimensions. */
   w = sprite->sw*z;
   h = sprite->sh*z;

   /* check if inbounds */
   if ((x < -w) || (x > SCREEN_W+w) ||
         (y < -h) || (y > SCREEN_H+h))
      return;

   /* texture coords */
   tx = sprite->sw*(double)(sx)/sprite->w;
   ty = sprite->sh*(sprite->sy-(double)sy-1)/sprite->h;

   gl_renderTexture( sprite, x, y, w, h,
         tx, ty, sprite->srw, sprite->srh, c, angle );
}

/**
 * @brief Blits a sprite, position is relative to the player with scaling and rotation.
 *
 * Since position is in "game coordinates" it is subject to all
 * sorts of position transformations.
 *
 *    @param sprite Sprite to blit.
 *    @param bx X position of the texture relative to the player.
 *    @param by Y position of the texture relative to the player.
 *    @param scalew Scaling of width.
 *    @param scaleh Scaling of height.
 *    @param angle Angle to rotate when rendering.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderSpriteScaleRotate( const glTexture* sprite,
      double bx, double by,
      double scalew, double scaleh, double angle,
      int sx, int sy, const glColour *c )
{
   double x,y, w,h, tx,ty, z;

   /* Translate coords. */
   z = cam_getZoom();
   gl_gameToScreenCoords( &x, &y, bx - sprite->sw*0.5, by - sprite->sh*0.5 );

   /* Scaled sprite dimensions. */
   w = sprite->sw*z*scalew;
   h = sprite->sh*z*scaleh;

   /* check if inbounds */
   if ((x < -w) || (x > SCREEN_W+w) ||
         (y < -h) || (y > SCREEN_H+h))
      return;

   /* texture coords */
   tx = sprite->sw*(double)(sx)/sprite->w;
   ty = sprite->sh*(sprite->sy-(double)sy-1)/sprite->h;

   gl_renderTexture( sprite, x, y, w, h,
         tx, ty, sprite->srw, sprite->srh, c, angle );
}

/**
 * @brief Blits a sprite interpolating, position is relative to the player.
 *
 * Since position is in "game coordinates" it is subject to all
 * sorts of position transformations.
 *
 * Interpolation is:  sa*inter + sb*1.-inter)
 *
 *    @param sa Sprite A to blit.
 *    @param sb Sprite B to blit.
 *    @param inter Amount to interpolate.
 *    @param bx X position of the texture relative to the player.
 *    @param by Y position of the texture relative to the player.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderSpriteInterpolate( const glTexture* sa, const glTexture *sb,
      double inter, double bx, double by,
      int sx, int sy, const glColour *c )
{
   gl_renderSpriteInterpolateScale( sa, sb, inter, bx, by, 1., 1., sx, sy, c );
}

/**
 * @brief Blits a sprite interpolating, position is relative to the player.
 *
 * Since position is in "game coordinates" it is subject to all
 * sorts of position transformations.
 *
 * Interpolation is:  sa*inter + sb*1.-inter)
 *
 *    @param sa Sprite A to blit.
 *    @param sb Sprite B to blit.
 *    @param inter Amount to interpolate.
 *    @param bx X position of the texture relative to the player.
 *    @param by Y position of the texture relative to the player.
 *    @param scalew X scale factor.
 *    @param scaleh Y scale factor.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderSpriteInterpolateScale( const glTexture* sa, const glTexture *sb,
      double inter, double bx, double by,
      double scalew, double scaleh,
      int sx, int sy, const glColour *c )
{
   double x,y, w,h, tx,ty, z;

   /* Translate coords. */
   gl_gameToScreenCoords( &x, &y, bx - scalew * sa->sw*0.5, by - scaleh * sa->sh*0.5 );

   /* Scaled sprite dimensions. */
   z = cam_getZoom();
   w = sa->sw*z*scalew;
   h = sa->sh*z*scaleh;

   /* check if inbounds */
   if ((x < -w) || (x > SCREEN_W+w) ||
         (y < -h) || (y > SCREEN_H+h))
      return;

   /* texture coords */
   tx = sa->sw*(double)(sx)/sa->w;
   ty = sa->sh*(sa->sy-(double)sy-1)/sa->h;

   gl_renderTextureInterpolate( sa, sb, inter, x, y, w, h,
         tx, ty, sa->srw, sa->srh, c );
}

/**
 * @brief Blits a sprite, position is in absolute screen coordinates.
 *
 *    @param sprite Sprite to blit.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderStaticSprite( const glTexture* sprite, double bx, double by,
      int sx, int sy, const glColour* c )
{
   double x,y, tx,ty;

   x = bx;
   y = by;

   /* texture coords */
   tx = sprite->sw*(double)(sx)/sprite->w;
   ty = sprite->sh*(sprite->sy-(double)sy-1)/sprite->h;

   /* actual blitting */
   gl_renderTexture( sprite, x, y, sprite->sw, sprite->sh,
         tx, ty, sprite->srw, sprite->srh, c, 0. );
}

/**
 * @brief Blits a sprite interpolating, position is relative to the player.
 *
 * Since position is in "game coordinates" it is subject to all
 * sorts of position transformations.
 *
 * Interpolation is:  sa*inter + sb*1.-inter)
 *
 *    @param sa Sprite A to blit.
 *    @param sb Sprite B to blit.
 *    @param inter Amount to interpolate.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderStaticSpriteInterpolate( const glTexture* sa, const glTexture *sb,
      double inter, double bx, double by,
      int sx, int sy, const glColour *c )
{
   gl_renderStaticSpriteInterpolateScale( sa, sb, inter, bx, by, 1., 1., sx, sy, c );
}

/**
 * @brief Blits a sprite interpolating, position is relative to the player.
 *
 * Since position is in "game coordinates" it is subject to all
 * sorts of position transformations.
 *
 * Interpolation is:  sa*inter + sb*1.-inter)
 *
 *    @param sa Sprite A to blit.
 *    @param sb Sprite B to blit.
 *    @param inter Amount to interpolate.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param scalew X scale factor.
 *    @param scaleh Y scale factor.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderStaticSpriteInterpolateScale( const glTexture* sa, const glTexture *sb,
      double inter, double bx, double by,
      double scalew, double scaleh,
      int sx, int sy, const glColour *c )
{
   double x,y, w,h, tx,ty;

   x = bx;
   y = by;

   /* Scaled sprite dimensions. */
   w = sa->sw*scalew;
   h = sa->sh*scaleh;

   /* check if inbounds */
   if ((x < -w) || (x > SCREEN_W+w) ||
         (y < -h) || (y > SCREEN_H+h))
      return;

   /* texture coords */
   tx = sa->sw*(double)(sx)/sa->w;
   ty = sa->sh*(sa->sy-(double)sy-1)/sa->h;

   gl_renderTextureInterpolate( sa, sb, inter, x, y, w, h,
         tx, ty, sa->srw, sa->srh, c );
}

/**
 * @brief Blits a scaled sprite, position is in absolute screen coordinates.
 *
 *    @param sprite Sprite to blit.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param bw Width of sprite to render at.
 *    @param bh Height of sprite to render at.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderScaleSprite( const glTexture* sprite,
      double bx, double by,
      int sx, int sy,
      double bw, double bh, const glColour* c )
{
   double x,y, tx,ty;

   x = bx;
   y = by;

   /* texture coords */
   tx = sprite->sw*(double)(sx)/sprite->w;
   ty = sprite->sh*(sprite->sy-(double)sy-1)/sprite->h;

   /* actual blitting */
   gl_renderTexture( sprite, x, y, bw, bh,
         tx, ty, sprite->srw, sprite->srh, c, 0. );
}

/**
 * @brief Blits a texture scaling it.
 *
 *    @param texture Texture to blit.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param bw Width to scale to.
 *    @param bh Height to scale to.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderScale( const glTexture* texture,
      double bx, double by,
      double bw, double bh, const glColour* c )
{
   double x,y, tx, ty;

   /* here we use absolute coords */
   x = bx;
   y = by;

   /* texture dimensions */
   tx = ty = 0.;

   /* Actual blitting. */
   gl_renderTexture( texture, x, y, bw, bh,
         tx, ty, texture->srw, texture->srh, c, 0. );
}

/**
 * @brief Blits a texture scaling it to fit a rectangle, but conserves aspect
 * ratio.
 *
 *    @param texture Texture to blit.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param bw Width to scale to.
 *    @param bh Height to scale to.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderScaleAspect( const glTexture* texture,
   double bx, double by, double bw, double bh,
   const glColour *c )
{
   double scale;
   double nw, nh;

   scale = MIN( bw / texture->w, bh / texture->h );

   nw = scale * texture->w;
   nh = scale * texture->h;

   bx += (bw-nw)*0.5;
   by += (bh-nh)*0.5;

   gl_renderScale( texture, bx, by, nw, nh, c );
}

/**
 * @brief Blits a texture to a position
 *
 *    @param texture Texture to blit.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderStatic( const glTexture* texture,
      double bx, double by, const glColour* c )
{
   double x,y;

   /* here we use absolute coords */
   x = bx;
   y = by;

   /* actual blitting */
   gl_renderTexture( texture, x, y, texture->sw, texture->sh,
         0., 0., texture->srw, texture->srh, c, 0. );
}

/**
 * @brief Renders a simple shader.
 *
 *    @param x X position.
 *    @param y Y position.
 *    @param w Width.
 *    @param h Height.
 *    @param r Rotation or 0. to disable.
 *    @param shd Shader to render.
 *    @param c Colour to use or NULL if not necessary.
 *    @param center Whether or not to center the shader on the position and use [-1,1] coordinates or set bottom-left and use [0,1] coordinates.
 */
void gl_renderShader( double x, double y, double w, double h, double r, const SimpleShader *shd, const glColour *c, int center )
{
   mat4 projection = gl_view_matrix;
   mat4_translate( &projection, x, y, 0. );
   if (r != 0.)
      mat4_rotate2d( &projection, r );
   mat4_scale( &projection, w, h, 1. );
   glUniform2f( shd->dimensions, w, h );
   gl_renderShaderH( shd, &projection, c, center );
}

/**
 * @brief Renders a simple shader with a transformation.
 *
 *    @param shd Shader to render.
 *    @param H Transformation matrix.
 *    @param c Colour to use or NULL if not necessary.
 *    @param center Whether or not to center the shader on the position and use [-1,1] coordinates or set bottom-left and use [0,1] coordinates.
 */
void gl_renderShaderH( const SimpleShader *shd, const mat4 *H, const glColour *c, int center )
{
   glEnableVertexAttribArray(shd->vertex);
   gl_vboActivateAttribOffset( center ? gl_circleVBO : gl_squareVBO, shd->vertex, 0, 2, GL_FLOAT, 0 );

   if (c != NULL)
      gl_uniformColour(shd->colour, c);

   gl_uniformMat4(shd->projection, H);

   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   glDisableVertexAttribArray(shd->vertex);
   glUseProgram(0);
   gl_checkErr();
}

/**
 * @brief Draws a circle.
 *
 *    @param cx X position of the center in screen coordinates.
 *    @param cy Y position of the center in screen coordinates.
 *    @param r Radius of the circle.
 *    @param c Colour to use.
 *    @param filled Whether or not it should be filled.
 */
void gl_renderCircle( double cx, double cy,
      double r, const glColour *c, int filled )
{
   /* Set the vertex. */
   mat4 projection = gl_view_matrix;
   mat4_translate( &projection, cx, cy, 0. );
   mat4_scale( &projection, r, r, 1. );

   /* Draw! */
   gl_renderCircleH( &projection, c, filled );
}

/**
 * @brief Draws a circle.
 *
 *    @param H Transformation matrix to draw the circle.
 *    @param c Colour to use.
 *    @param filled Whether or not it should be filled.
 */
void gl_renderCircleH( const mat4 *H, const glColour *c, int filled )
{
   // TODO handle shearing and different x/y scaling
   GLfloat r = H->m[0][0] / gl_view_matrix.m[0][0];

   glUseProgram( shaders.circle.program );
   glUniform2f( shaders.circle.dimensions, r, r );
   glUniform1i( shaders.circle.parami, filled );
   gl_renderShaderH( &shaders.circle, H, c, 1 );
}

/**
 * @brief Draws a line.
 *
 *    @param x1 X position of the first point in screen coordinates.
 *    @param y1 Y position of the first point in screen coordinates.
 *    @param x2 X position of the second point in screen coordinates.
 *    @param y2 Y position of the second point in screen coordinates.
 *    @param c Colour to use.
 */
void gl_renderLine( double x1, double y1,
      double x2, double y2, const glColour *c )
{
   double a = atan2( y2-y1, x2-x1 );
   double s = hypotf( x2-x1, y2-y1 );

   glUseProgram(shaders.sdfsolid.program);
   glUniform1f(shaders.sdfsolid.paramf, 1.); /* No outline. */
   gl_renderShader( (x1+x2)*0.5, (y1+y2)*0.5, s*0.5+0.5, 1.0, a, &shaders.sdfsolid, c, 1 );
}

/**
 * @brief Sets up 2d clipping planes around a rectangle.
 *
 *    @param x X position of the rectangle.
 *    @param y Y position of the rectangle.
 *    @param w Width of the rectangle.
 *    @param h Height of the rectangle.
 */
void gl_clipRect( int x, int y, int w, int h )
{
   double rx, ry, rw, rh;
   rx = (x + gl_screen.x) / gl_screen.mxscale;
   ry = (y + gl_screen.y) / gl_screen.myscale;
   rw = w / gl_screen.mxscale;
   rh = h / gl_screen.myscale;
   glScissor( rx, ry, rw, rh );
   glEnable( GL_SCISSOR_TEST );
}

/**
 * @brief Clears the 2d clipping planes.
 */
void gl_unclipRect (void)
{
   glDisable( GL_SCISSOR_TEST );
   glScissor( 0, 0, gl_screen.rw, gl_screen.rh );
}

/**
 * @brief Initializes the OpenGL rendering routines.
 *
 *    @return 0 on success.
 */
int gl_initRender (void)
{
   GLfloat vertex[10];

   /* Initialize the VBO. */
   gl_renderVBO = gl_vboCreateStream( sizeof(GLfloat) *
         OPENGL_RENDER_VBO_SIZE*(2 + 2 + 4), NULL );
   gl_renderVBOtexOffset = sizeof(GLfloat) * OPENGL_RENDER_VBO_SIZE*2;
   gl_renderVBOcolOffset = sizeof(GLfloat) * OPENGL_RENDER_VBO_SIZE*(2+2);

   vertex[0] = 0.;
   vertex[1] = 0.;
   vertex[2] = 1.;
   vertex[3] = 0.;
   vertex[4] = 0.;
   vertex[5] = 1.;
   vertex[6] = 1.;
   vertex[7] = 1.;
   gl_squareVBO = gl_vboCreateStatic( sizeof(GLfloat) * 8, vertex );

   vertex[0] = -1.;
   vertex[1] = -1.;
   vertex[2] = 1.;
   vertex[3] = -1.;
   vertex[4] = -1.;
   vertex[5] = 1.;
   vertex[6] = 1.;
   vertex[7] = 1.;
   gl_circleVBO = gl_vboCreateStatic( sizeof(GLfloat) * 8, vertex );

   vertex[0] = 0.;
   vertex[1] = 0.;
   vertex[2] = 1.;
   vertex[3] = 0.;
   vertex[4] = 1.;
   vertex[5] = 1.;
   vertex[6] = 0.;
   vertex[7] = 1.;
   vertex[8] = 0.;
   vertex[9] = 0.;
   gl_squareEmptyVBO = gl_vboCreateStatic( sizeof(GLfloat) * 8, vertex );

   vertex[0] = 0.;
   vertex[1] = 0.;
   vertex[2] = 1.;
   vertex[3] = 0.;
   gl_lineVBO = gl_vboCreateStatic( sizeof(GLfloat) * 4, vertex );

   vertex[0] = 0.5*cos(4.*M_PI/3.);
   vertex[1] = 0.5*sin(4.*M_PI/3.);
   vertex[2] = 0.5*cos(0.);
   vertex[3] = 0.5*sin(0.);
   vertex[4] = 0.5*cos(2.*M_PI/3.);
   vertex[5] = 0.5*sin(2.*M_PI/3.);
   vertex[6] = vertex[0];
   vertex[7] = vertex[1];
   gl_triangleVBO = gl_vboCreateStatic( sizeof(GLfloat) * 8, vertex );

   gl_checkErr();

   return 0;
}

/**
 * @brief Cleans up the OpenGL rendering routines.
 */
void gl_exitRender (void)
{
   /* Destroy the VBO. */
   gl_vboDestroy( gl_renderVBO );
   gl_vboDestroy( gl_squareVBO );
   gl_vboDestroy( gl_circleVBO );
   gl_vboDestroy( gl_squareEmptyVBO );
   gl_vboDestroy( gl_lineVBO );
   gl_vboDestroy( gl_triangleVBO );
   gl_renderVBO = NULL;
}
