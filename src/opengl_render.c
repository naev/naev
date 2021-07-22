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
static gl_vbo *gl_crossVBO = 0;
static gl_vbo *gl_lineVBO = 0;
static gl_vbo *gl_triangleVBO = 0;
static int gl_renderVBOtexOffset = 0; /**< VBO texture offset. */
static int gl_renderVBOcolOffset = 0; /**< VBO colour offset. */

/*
 * prototypes
 */

void gl_beginSolidProgram(gl_Matrix4 projection, const glColour *c)
{
   glUseProgram(shaders.solid.program);
   glEnableVertexAttribArray(shaders.solid.vertex);
   gl_uniformColor(shaders.solid.color, c);
   gl_Matrix4_Uniform(shaders.solid.projection, projection);
}

void gl_endSolidProgram (void)
{
   glDisableVertexAttribArray(shaders.solid.vertex);
   glUseProgram(0);
   gl_checkErr();
}


void gl_beginSmoothProgram(gl_Matrix4 projection)
{
   glUseProgram(shaders.smooth.program);
   glEnableVertexAttribArray(shaders.smooth.vertex);
   glEnableVertexAttribArray(shaders.smooth.vertex_color);
   gl_Matrix4_Uniform(shaders.smooth.projection, projection);
}

void gl_endSmoothProgram() {
   glDisableVertexAttribArray(shaders.smooth.vertex);
   glDisableVertexAttribArray(shaders.smooth.vertex_color);
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
   gl_Matrix4 projection;

   /* Set the vertex. */
   projection = gl_view_matrix;
   projection = gl_Matrix4_Translate(projection, x, y, 0);
   projection = gl_Matrix4_Scale(projection, w, h, 1);

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
   gl_Matrix4 projection;

   projection = gl_view_matrix;
   projection = gl_Matrix4_Translate(projection, x, y, 0);
   projection = gl_Matrix4_Scale(projection, w, h, 1);

   gl_renderRectH( &projection, c, 0 );
}


/**
 * @brief Renders a rectangle.
 *
 *    @param H Transformation matrix to apply.
 *    @param filled Whether or not to fill.
 *    @param c Rectangle colour.
 */
void gl_renderRectH( const gl_Matrix4 *H, const glColour *c, int filled )
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
 * @brief Renders an OK / Not OK status (green circle or slashed red circle).
 *
 *    @param x X position to render rectangle at.
 *    @param y Y position to render rectangle at.
 *    @param w Rectangle width.
 *    @param h Rectangle height.
 *    @param ok Boolean to represent with the drawing.
 */
void gl_renderStatus( double x, double y, double w, double h, int ok )
{
   gl_Matrix4 projection;

   projection = gl_view_matrix;
   projection = gl_Matrix4_Translate( projection, x + w/2, y + h/2, 0 );
   projection = gl_Matrix4_Scale( projection, w/2, h/2, 1 );

   glUseProgram( shaders.status.program );
   gl_Matrix4_Uniform( shaders.status.projection, projection );
   glUniform1f( shaders.status.ok, ok );
   glEnableVertexAttribArray( shaders.status.vertex );
   gl_vboActivateAttribOffset( gl_circleVBO, shaders.status.vertex, 0, 2, GL_FLOAT, 0 );
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
   glDisableVertexAttribArray( shaders.status.vertex );
   glUseProgram(0);
   gl_checkErr();
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
   gl_Matrix4 projection;

   projection = gl_Matrix4_Translate(gl_view_matrix, x, y, 0);
   projection = gl_Matrix4_Scale(projection, r, r, 1);

   gl_beginSolidProgram(projection, c);
   gl_vboActivateAttribOffset( gl_crossVBO, shaders.solid.vertex, 0, 2, GL_FLOAT, 0 );
   glDrawArrays( GL_LINES, 0, 4 );
   gl_endSolidProgram();
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
   gl_Matrix4 projection;

   projection = gl_Matrix4_Translate(gl_view_matrix, x, y, 0);
   if (a != 0.)
      projection = gl_Matrix4_Rotate2d(projection, a);
   projection = gl_Matrix4_Scale(projection, s*length, s, 1.);

   gl_beginSolidProgram(projection, c);
   gl_vboActivateAttribOffset( gl_triangleVBO, shaders.solid.vertex, 0, 2, GL_FLOAT, 0 );
   glDrawArrays( GL_LINE_STRIP, 0, 4 );
   gl_endSolidProgram();
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
void gl_blitTexture(  const glTexture* texture,
      const double x, const double y,
      const double w, const double h,
      const double tx, const double ty,
      const double tw, const double th, const glColour *c, const double angle )
{
   // Half width and height
   double hw, hh;
   gl_Matrix4 projection, tex_mat;

   glUseProgram(shaders.texture.program);

   /* Bind the texture. */
   glBindTexture( GL_TEXTURE_2D, texture->texture);

   /* Must have colour for now. */
   if (c == NULL)
      c = &cWhite;

   hw = w/2.;
   hh = h/2.;

   /* Set the vertex. */
   projection = gl_view_matrix;
   if (angle==0.){
     projection = gl_Matrix4_Translate(projection, x, y, 0);
     projection = gl_Matrix4_Scale(projection, w, h, 1);
   } else {
     projection = gl_Matrix4_Translate(projection, x+hw, y+hh, 0);
     projection = gl_Matrix4_Rotate2d(projection, angle);
     projection = gl_Matrix4_Translate(projection, -hw, -hh, 0);
     projection = gl_Matrix4_Scale(projection, w, h, 1);
   }
   glEnableVertexAttribArray( shaders.texture.vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.texture.vertex,
         0, 2, GL_FLOAT, 0 );

   /* Set the texture. */
   tex_mat = (texture->flags & OPENGL_TEX_VFLIP) ? gl_Matrix4_Ortho(-1, 1, 2, 0, 1, -1) : gl_Matrix4_Identity();
   tex_mat = gl_Matrix4_Translate(tex_mat, tx, ty, 0);
   tex_mat = gl_Matrix4_Scale(tex_mat, tw, th, 1);

   /* Set shader uniforms. */
   gl_uniformColor(shaders.texture.color, c);
   gl_Matrix4_Uniform(shaders.texture.projection, projection);
   gl_Matrix4_Uniform(shaders.texture.tex_mat, tex_mat);

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shaders.texture.vertex );

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
void gl_blitTextureInterpolate(  const glTexture* ta,
      const glTexture* tb, const double inter,
      const double x, const double y,
      const double w, const double h,
      const double tx, const double ty,
      const double tw, const double th, const glColour *c )
{
   /* No interpolation. */
   if (tb == NULL) {
      gl_blitTexture( ta, x, y, w, h, tx, ty, tw, th, c, 0. );
      return;
   }

   /* Corner cases. */
   if (inter == 1.) {
      gl_blitTexture( ta, x, y, w, h, tx, ty, tw, th, c, 0. );
      return;
   }
   else if (inter == 0.) {
      gl_blitTexture( tb, x, y, w, h, tx, ty, tw, th, c, 0. );
      return;
   }

   gl_Matrix4 projection, tex_mat;

   glUseProgram(shaders.texture_interpolate.program);

   /* Bind the textures. */
   glActiveTexture( GL_TEXTURE0 );
   glBindTexture( GL_TEXTURE_2D, ta->texture);
   glActiveTexture( GL_TEXTURE1 );
   glBindTexture( GL_TEXTURE_2D, tb->texture);

   /* Must have colour for now. */
   if (c == NULL)
      c = &cWhite;

   /* Set the vertex. */
   projection = gl_view_matrix;
   projection = gl_Matrix4_Translate(projection, x, y, 0);
   projection = gl_Matrix4_Scale(projection, w, h, 1);
   glEnableVertexAttribArray( shaders.texture_interpolate.vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.texture_interpolate.vertex, 0, 2, GL_FLOAT, 0 );

   /* Set the texture. */
   tex_mat = (ta->flags & OPENGL_TEX_VFLIP) ? gl_Matrix4_Ortho(-1, 1, 2, 0, 1, -1) : gl_Matrix4_Identity();
   tex_mat = gl_Matrix4_Translate(tex_mat, tx, ty, 0);
   tex_mat = gl_Matrix4_Scale(tex_mat, tw, th, 1);

   /* Set shader uniforms. */
   glUniform1i(shaders.texture_interpolate.sampler1, 0);
   glUniform1i(shaders.texture_interpolate.sampler2, 1);
   gl_uniformColor(shaders.texture_interpolate.color, c);
   glUniform1f(shaders.texture_interpolate.inter, inter);
   gl_Matrix4_Uniform(shaders.texture_interpolate.projection, projection);
   gl_Matrix4_Uniform(shaders.texture_interpolate.tex_mat, tex_mat);

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shaders.texture_interpolate.vertex );
   glActiveTexture( GL_TEXTURE0 );

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
   *nx = (bx - cx) * z + gx + SCREEN_W/2.;
   *ny = (by - cy) * z + gy + SCREEN_H/2.;
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
   *nx = (bx - SCREEN_W/2. - gx) / z + cx;
   *ny = (by - SCREEN_H/2. - gy) / z + cy;
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
void gl_blitSprite( const glTexture* sprite, const double bx, const double by,
      const int sx, const int sy, const glColour* c )
{
   double x,y, w,h, tx,ty, z;

   /* Translate coords. */
   z = cam_getZoom();
   gl_gameToScreenCoords( &x, &y, bx - sprite->sw/2., by - sprite->sh/2. );

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

   gl_blitTexture( sprite, x, y, w, h,
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
 *    @param bx X position of the texture relative to the player.
 *    @param by Y position of the texture relative to the player.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_blitSpriteInterpolate( const glTexture* sa, const glTexture *sb,
      double inter, const double bx, const double by,
      const int sx, const int sy, const glColour *c )
{
   gl_blitSpriteInterpolateScale( sa, sb, inter, bx, by, 1., 1., sx, sy, c );
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
void gl_blitSpriteInterpolateScale( const glTexture* sa, const glTexture *sb,
      double inter, const double bx, const double by,
      double scalew, double scaleh,
      const int sx, const int sy, const glColour *c )
{
   double x,y, w,h, tx,ty, z;

   /* Translate coords. */
   gl_gameToScreenCoords( &x, &y, bx - scalew * sa->sw/2., by - scaleh * sa->sh/2. );

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

   gl_blitTextureInterpolate( sa, sb, inter, x, y, w, h,
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
void gl_blitStaticSprite( const glTexture* sprite, const double bx, const double by,
      const int sx, const int sy, const glColour* c )
{
   double x,y, tx,ty;

   x = bx;
   y = by;

   /* texture coords */
   tx = sprite->sw*(double)(sx)/sprite->w;
   ty = sprite->sh*(sprite->sy-(double)sy-1)/sprite->h;

   /* actual blitting */
   gl_blitTexture( sprite, x, y, sprite->sw, sprite->sh,
         tx, ty, sprite->srw, sprite->srh, c, 0. );
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
void gl_blitScaleSprite( const glTexture* sprite,
      const double bx, const double by,
      const int sx, const int sy,
      const double bw, const double bh, const glColour* c )
{
   double x,y, tx,ty;

   x = bx;
   y = by;

   /* texture coords */
   tx = sprite->sw*(double)(sx)/sprite->w;
   ty = sprite->sh*(sprite->sy-(double)sy-1)/sprite->h;

   /* actual blitting */
   gl_blitTexture( sprite, x, y, bw, bh,
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
void gl_blitScale( const glTexture* texture,
      const double bx, const double by,
      const double bw, const double bh, const glColour* c )
{
   double x,y;
   double tx,ty;

   /* here we use absolute coords */
   x = bx;
   y = by;

   /* texture dimensions */
   tx = ty = 0.;

   /* Actual blitting. */
   gl_blitTexture( texture, x, y, bw, bh,
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
void gl_blitScaleAspect( const glTexture* texture,
   double bx, double by, double bw, double bh,
   const glColour *c )
{
   double scale;
   double nw, nh;

   scale = MIN( bw / texture->w, bh / texture->h );

   nw = scale * texture->w;
   nh = scale * texture->h;

   bx += (bw-nw)/2.;
   by += (bh-nh)/2.;

   gl_blitScale( texture, bx, by, nw, nh, c );
}


/**
 * @brief Blits a texture to a position
 *
 *    @param texture Texture to blit.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_blitStatic( const glTexture* texture,
      const double bx, const double by, const glColour* c )
{
   double x,y;

   /* here we use absolute coords */
   x = bx;
   y = by;

   /* actual blitting */
   gl_blitTexture( texture, x, y, texture->sw, texture->sh,
         0., 0., texture->srw, texture->srh, c, 0. );
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
void gl_drawCircle( const double cx, const double cy,
      const double r, const glColour *c, int filled )
{
   gl_Matrix4 projection;

   /* Set the vertex. */
   projection = gl_view_matrix;
   projection = gl_Matrix4_Translate(projection, cx, cy, 0);
   projection = gl_Matrix4_Scale(projection, r, r, 1);

   /* Draw! */
   gl_drawCircleH( &projection, c, filled );
}


/**
 * @brief Draws a circle.
 *
 *    @param H Transformation matrix to draw the circle.
 *    @param c Colour to use.
 *    @param filled Whether or not it should be filled.
 */
void gl_drawCircleH( const gl_Matrix4 *H, const glColour *c, int filled )
{
   // TODO handle shearing and different x/y scaling
   GLfloat r = H->m[0][0] / gl_view_matrix.m[0][0];

   if (filled) {
      glUseProgram( shaders.circle_filled.program );

      glEnableVertexAttribArray( shaders.circle_filled.vertex );
      gl_vboActivateAttribOffset( gl_circleVBO, shaders.circle_filled.vertex,
            0, 2, GL_FLOAT, 0 );

      /* Set shader uniforms. */
      gl_uniformColor( shaders.circle_filled.color, c );
      gl_Matrix4_Uniform( shaders.circle_filled.projection, *H );
      glUniform1f( shaders.circle_filled.radius, r );

      /* Draw. */
      glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

      /* Clear state. */
      glDisableVertexAttribArray( shaders.circle_filled.vertex );
   }
   else {
      glUseProgram( shaders.circle.program );

      glEnableVertexAttribArray( shaders.circle.vertex );
      gl_vboActivateAttribOffset( gl_circleVBO, shaders.circle.vertex,
            0, 2, GL_FLOAT, 0 );

      /* Set shader uniforms. */
      gl_uniformColor( shaders.circle.color, c );
      gl_Matrix4_Uniform( shaders.circle.projection, *H );
      glUniform1f( shaders.circle.radius, r );

      /* Draw. */
      glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

      /* Clear state. */
      glDisableVertexAttribArray( shaders.circle.vertex );
   }
   glUseProgram(0);

   /* Check errors. */
   gl_checkErr();
}


/**
 * @brief Draws a partial circle.
 *
 *    @param cx X position of the center in screen coordinates.
 *    @param cy Y position of the center in screen coordinates.
 *    @param r Radius of the circle.
 *    @param c Colour to use.
 *    @param angle Starting angle in radians.
 *    @param arc Length of the arc (0 to 2 pi)
 */
void gl_drawCirclePartial( const double cx, const double cy,
      const double r, const glColour *c, double angle, double arc )
{
   gl_Matrix4 projection;

   /* Set the vertex. */
   projection = gl_view_matrix;
   projection = gl_Matrix4_Translate(projection, cx, cy, 0);
   projection = gl_Matrix4_Scale(projection, r, r, 1);

   /* Draw! */
   gl_drawCirclePartialH( &projection, c, angle, arc );
}


/**
 * @brief Draws a partial circle.
 *
 *    @param H Transformation matrix to draw the circle.
 *    @param c Colour to use.
 *    @param angle Starting angle in radians.
 *    @param arc Length of the arc (0 to 2 pi)
 */
void gl_drawCirclePartialH( const gl_Matrix4 *H, const glColour *c, double angle, double arc )
{
   // TODO handle shearing and different x/y scaling
   GLfloat r = H->m[0][0] / gl_view_matrix.m[0][0];

   /* Draw. */
   glUseProgram( shaders.circle_partial.program );

   glEnableVertexAttribArray( shaders.circle_partial.vertex );
   gl_vboActivateAttribOffset( gl_circleVBO, shaders.circle_partial.vertex,
         0, 2, GL_FLOAT, 0 );

   /* Set shader uniforms. */
   gl_uniformColor( shaders.circle_partial.color, c );
   gl_Matrix4_Uniform( shaders.circle_partial.projection, *H );
   glUniform1f( shaders.circle_partial.radius, r );
   glUniform1f( shaders.circle_partial.angle1, angle );
   glUniform1f( shaders.circle_partial.angle2, arc );

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shaders.circle_partial.vertex );

   glUseProgram(0);
   gl_checkErr();
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
void gl_drawLine( const double x1, const double y1,
      const double x2, const double y2, const glColour *c )
{
   gl_Matrix4 projection;
   double a, s;

   a = atan2( y2-y1, x2-x1 );
   s = hypotf( x2-x1, y2-y1 );

   projection = gl_view_matrix;

   projection = gl_Matrix4_Translate(projection, x1, y1, 0);
   projection = gl_Matrix4_Rotate2d(projection, a);
   projection = gl_Matrix4_Scale(projection, s, s, 1);

   gl_beginSolidProgram(projection, c);
   gl_vboActivateAttribOffset( gl_lineVBO, shaders.solid.vertex, 0, 2, GL_FLOAT, 0 );
   glDrawArrays( GL_LINES, 0, 2 );
   gl_endSolidProgram();
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
   vertex[1] = -1.;
   vertex[2] = 0.;
   vertex[3] = 1.;
   vertex[4] = -1.;
   vertex[5] = 0.;
   vertex[6] = 1.;
   vertex[7] = 0.;
   gl_crossVBO = gl_vboCreateStatic( sizeof(GLfloat) * 8, vertex );

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
   gl_vboDestroy( gl_crossVBO );
   gl_vboDestroy( gl_lineVBO );
   gl_vboDestroy( gl_triangleVBO );
   gl_renderVBO = NULL;
}
