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
 *  * Origin (0., 0.) wouldbe ontop of the player.
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


#include "opengl.h"

#include "naev.h"

#include "log.h"
#include "ndata.h"
#include "gui.h"
#include "conf.h"


#define OPENGL_RENDER_VBO_SIZE      256 /**< Size of VBO. */


static Vector2d* gl_camera  = NULL; /**< Camera we are using. */
static double gl_cameraZ    = 1.; /**< Current in-game zoom. */
static double gl_cameraX    = 0.; /**< X position of camera. */
static double gl_cameraY    = 0.; /**< Y position of camera. */
static gl_vbo *gl_renderVBO = 0; /**< VBO for rendering stuff. */
static int gl_renderVBOtexOffset = 0; /**< VBO texture offset. */
static int gl_renderVBOcolOffset = 0; /**< VBO colour offset. */


/*
 * Circle textures.
 */
static glTexture *gl_circle      = NULL; /**< Circle mipmap. */


/*
 * prototypes
 */
static void gl_drawCircleEmpty( const double cx, const double cy,
      const double r, const glColour *c );
static glTexture *gl_genCircle( int radius );
static void gl_blitTextureInterpolate(  const glTexture* ta,
      const glTexture* tb, const double inter,
      const double x, const double y,
      const double w, const double h,
      const double tx, const double ty,
      const double tw, const double th, const glColour *c );


/**
 * @brief Sets the camera zoom.
 *
 * This is the zoom used in game coordinates.
 *
 *    @param zoom Zoom to set to.
 */
void gl_cameraZoom( double zoom )
{
   gl_cameraZ = zoom;
}


/**
 * @brief Gets the camera zoom.
 *
 *    @param zoom Stores the camera zoom.
 */
void gl_cameraZoomGet( double * zoom )
{
   *zoom = gl_cameraZ;
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
   GLfloat vertex[4*2], col[4*4];

   /* Set the vertex. */
   /*   1--2
    *   |  |
    *   3--4
    */
   vertex[0] = (GLfloat)x;
   vertex[4] = vertex[0];
   vertex[2] = vertex[0] + (GLfloat)w;
   vertex[6] = vertex[2];
   vertex[1] = (GLfloat)y;
   vertex[3] = vertex[1];
   vertex[5] = vertex[1] + (GLfloat)h;
   vertex[7] = vertex[5];
   gl_vboSubData( gl_renderVBO, 0, 4*2*sizeof(GLfloat), vertex );
   gl_vboActivateOffset( gl_renderVBO, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );

   /* Set the colour. */
   col[0] = c->r;
   col[1] = c->g;
   col[2] = c->b;
   col[3] = c->a;
   col[4] = col[0];
   col[5] = col[1];
   col[6] = col[2];
   col[7] = col[3];
   col[8] = col[0];
   col[9] = col[1];
   col[10] = col[2];
   col[11] = col[3];
   col[12] = col[0];
   col[13] = col[1];
   col[14] = col[2];
   col[15] = col[3];
   gl_vboSubData( gl_renderVBO, gl_renderVBOcolOffset, 4*4*sizeof(GLfloat), col );
   gl_vboActivateOffset( gl_renderVBO, GL_COLOR_ARRAY,
         gl_renderVBOcolOffset, 4, GL_FLOAT, 0 );

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   gl_vboDeactivate();

   /* Check errors. */
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
void gl_renderRectEmpty( double x, double y, double w, double h, const glColour *c )
{
   GLfloat vx, vy, vxw, vyh;
   GLfloat vertex[5*2], col[5*4];

   /* Helper variables. */
   vx  = (GLfloat) x;
   vy  = (GLfloat) y;
   vxw = vx + (GLfloat) w;
   vyh = vy + (GLfloat) h;

   /* Set the vertex. */
   vertex[0] = vx;
   vertex[1] = vy;
   vertex[2] = vxw;
   vertex[3] = vy;
   vertex[4] = vxw;
   vertex[5] = vyh;
   vertex[6] = vx;
   vertex[7] = vyh;
   vertex[8] = vx;
   vertex[9] = vy;
   gl_vboSubData( gl_renderVBO, 0, sizeof(vertex), vertex );
   gl_vboActivateOffset( gl_renderVBO, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );

   /* Set the colour. */
   col[0] = c->r;
   col[1] = c->g;
   col[2] = c->b;
   col[3] = c->a;
   col[4] = col[0];
   col[5] = col[1];
   col[6] = col[2];
   col[7] = col[3];
   col[8] = col[0];
   col[9] = col[1];
   col[10] = col[2];
   col[11] = col[3];
   col[12] = col[0];
   col[13] = col[1];
   col[14] = col[2];
   col[15] = col[3];
   col[16] = col[0];
   col[17] = col[1];
   col[18] = col[2];
   col[19] = col[3];
   gl_vboSubData( gl_renderVBO, gl_renderVBOcolOffset, sizeof(col), col );
   gl_vboActivateOffset( gl_renderVBO, GL_COLOR_ARRAY,
         gl_renderVBOcolOffset, 4, GL_FLOAT, 0 );

   /* Draw. */
   glDrawArrays( GL_LINE_STRIP, 0, 5 );

   /* Clear state. */
   gl_vboDeactivate();

   /* Check errors. */
   gl_checkErr();
}


/**
 * @brief Texture blitting backend.
 *
 *    @param texture Texture to blit.
 *    @param x X position of the texture on the screen.
 *    @param y Y position of the texture on the screen.
 *    @param tx X position within the texture.
 *    @param ty Y position within the texture.
 *    @param tw Texture width.
 *    @param th Texture height.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_blitTexture(  const glTexture* texture,
      const double x, const double y,
      const double w, const double h,
      const double tx, const double ty,
      const double tw, const double th, const glColour *c )
{
   GLfloat vertex[4*2], tex[4*2], col[4*4];

   /* Bind the texture. */
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, texture->texture);

   /* Must have colour for now. */
   if (c == NULL)
      c = &cWhite;

   /* Set the vertex. */
   vertex[0] = (GLfloat)x;
   vertex[4] = vertex[0];
   vertex[2] = vertex[0] + (GLfloat)w;
   vertex[6] = vertex[2];
   vertex[1] = (GLfloat)y;
   vertex[3] = vertex[1];
   vertex[5] = vertex[1] + (GLfloat)h;
   vertex[7] = vertex[5];
   gl_vboSubData( gl_renderVBO, 0, 4*2*sizeof(GLfloat), vertex );
   gl_vboActivateOffset( gl_renderVBO, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );

   /* Set the texture. */
   tex[0] = (GLfloat)tx;
   tex[4] = tex[0];
   tex[2] = tex[0] + (GLfloat)tw;
   tex[6] = tex[2];
   tex[1] = (GLfloat)ty;
   tex[3] = tex[1];
   tex[5] = tex[1] + (GLfloat)th;
   tex[7] = tex[5];
   gl_vboSubData( gl_renderVBO, gl_renderVBOtexOffset, 4*2*sizeof(GLfloat), tex );
   gl_vboActivateOffset( gl_renderVBO, GL_TEXTURE_COORD_ARRAY,
         gl_renderVBOtexOffset, 2, GL_FLOAT, 0 );

   /* Set the colour. */
   col[0] = c->r;
   col[1] = c->g;
   col[2] = c->b;
   col[3] = c->a;
   col[4] = col[0];
   col[5] = col[1];
   col[6] = col[2];
   col[7] = col[3];
   col[8] = col[0];
   col[9] = col[1];
   col[10] = col[2];
   col[11] = col[3];
   col[12] = col[0];
   col[13] = col[1];
   col[14] = col[2];
   col[15] = col[3];
   gl_vboSubData( gl_renderVBO, gl_renderVBOcolOffset, 4*4*sizeof(GLfloat), col );
   gl_vboActivateOffset( gl_renderVBO, GL_COLOR_ARRAY,
         gl_renderVBOcolOffset, 4, GL_FLOAT, 0 );

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   gl_vboDeactivate();
   glDisable(GL_TEXTURE_2D);

   /* anything failed? */
   gl_checkErr();
}


/**
 * @brief Texture blitting backend for interpolated texture.
 *
 * Value blitted is  ta*inter + tb*(1.-inter).
 *
 *    @param ta Texture A to blit.
 *    @param tb Texture B to blit.
 *    @param intere Amount of interpolation to do.
 *    @param x X position of the texture on the screen.
 *    @param y Y position of the texture on the screen.
 *    @param tx X position within the texture.
 *    @param ty Y position within the texture.
 *    @param tw Texture width.
 *    @param th Texture height.
 *    @param c Colour to use (modifies texture colour).
 */
static void gl_blitTextureInterpolate(  const glTexture* ta,
      const glTexture* tb, const double inter,
      const double x, const double y,
      const double w, const double h,
      const double tx, const double ty,
      const double tw, const double th, const glColour *c )
{
   GLfloat vertex[4*2], tex[4*2], col[4*4];
   GLfloat mcol[4] = { 0., 0., 0. };

   /* No interpolation. */
   if (!conf.interpolate || (tb == NULL)) {
      gl_blitTexture( ta, x, y, w, h, tx, ty, tw, th, c );
      return;
   }

   /* Corner cases. */
   if (inter == 1.) {
      gl_blitTexture( ta, x, y, w, h, tx, ty, tw, th, c );
      return;
   }
   else if (inter == 0.) {
      gl_blitTexture( tb, x, y, w, h, tx, ty, tw, th, c );
      return;
   }

   /* No multitexture. */
   if (nglActiveTexture == NULL) {
      if (inter > 0.5)
         gl_blitTexture( ta, x, y, w, h, tx, ty, tw, th, c );
      else
         gl_blitTexture( tb, x, y, w, h, tx, ty, tw, th, c );
   }

   /* Set default colour. */
   if (c == NULL)
      c = &cWhite;

   /* Bind the textures. */
   /* Texture 0. */
   nglActiveTexture( GL_TEXTURE0 );
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, ta->texture);

   /* Set the mode. */
   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );

   /* Interpolate texture and alpha. */
   glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_INTERPOLATE );
   glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_INTERPOLATE );
   mcol[3] = inter;
   glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, mcol );

   /* Arguments. */
   /* Arg0. */
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB,    GL_TEXTURE0 );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB,   GL_SRC_COLOR );
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA,  GL_TEXTURE0 );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA );
   /* Arg1. */
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB,    GL_TEXTURE1 );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB,   GL_SRC_COLOR );
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_ALPHA,  GL_TEXTURE1 );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA );
   /* Arg2. */
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_RGB,    GL_CONSTANT );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND2_RGB,   GL_SRC_ALPHA );
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_ALPHA,  GL_CONSTANT );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA );

   /* Texture 1. */
   nglActiveTexture( GL_TEXTURE1 );
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, tb->texture);

   /* Set the mode. */
   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );

   /* Interpolate texture and alpha. */
   glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_MODULATE );
   glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_MODULATE );

   /* Arguments. */
   /* Arg0. */
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB,    GL_PREVIOUS );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB,   GL_SRC_COLOR );
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA,  GL_PREVIOUS );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA );
   /* Arg1. */
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB,    GL_PRIMARY_COLOR );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB,   GL_SRC_COLOR );
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_ALPHA,  GL_PRIMARY_COLOR );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA );

   /* Set the colour. */
   col[0] = c->r;
   col[1] = c->g;
   col[2] = c->b;
   col[3] = c->a;
   col[4] = col[0];
   col[5] = col[1];
   col[6] = col[2];
   col[7] = col[3];
   col[8] = col[0];
   col[9] = col[1];
   col[10] = col[2];
   col[11] = col[3];
   col[12] = col[0];
   col[13] = col[1];
   col[14] = col[2];
   col[15] = col[3];
   gl_vboSubData( gl_renderVBO, gl_renderVBOcolOffset, 4*4*sizeof(GLfloat), col );
   gl_vboActivateOffset( gl_renderVBO, GL_COLOR_ARRAY,
         gl_renderVBOcolOffset, 4, GL_FLOAT, 0 );

   /* Set the vertex. */
   vertex[0] = (GLfloat)x;
   vertex[4] = vertex[0];
   vertex[2] = vertex[0] + (GLfloat)w;
   vertex[6] = vertex[2];
   vertex[1] = (GLfloat)y;
   vertex[3] = vertex[1];
   vertex[5] = vertex[1] + (GLfloat)h;
   vertex[7] = vertex[5];
   gl_vboSubData( gl_renderVBO, 0, 4*2*sizeof(GLfloat), vertex );
   gl_vboActivateOffset( gl_renderVBO, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );

   /* Set the texture. */
   tex[0] = (GLfloat)tx;
   tex[4] = tex[0];
   tex[2] = tex[0] + (GLfloat)tw;
   tex[6] = tex[2];
   tex[1] = (GLfloat)ty;
   tex[3] = tex[1];
   tex[5] = tex[1] + (GLfloat)th;
   tex[7] = tex[5];
   gl_vboSubData( gl_renderVBO, gl_renderVBOtexOffset, 4*2*sizeof(GLfloat), tex );
   gl_vboActivateOffset( gl_renderVBO, GL_TEXTURE0,
         gl_renderVBOtexOffset, 2, GL_FLOAT, 0 );
   gl_vboActivateOffset( gl_renderVBO, GL_TEXTURE1,
         gl_renderVBOtexOffset, 2, GL_FLOAT, 0 );

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   gl_vboDeactivate();
   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
   glDisable(GL_TEXTURE_2D);
   nglActiveTexture( GL_TEXTURE0 );
   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
   glDisable(GL_TEXTURE_2D);

   /* anything failed? */
   gl_checkErr();
}


/**
 * @brief Convertes ingame coordinates to screen coordinates.
 *
 *    @param[out] nx New screen X coord.
 *    @param[out] ny New screen Y coord.
 *    @param bx Game X coord to translate.
 *    @param by Game Y coord to translate.
 */
void gl_gameToScreenCoords( double *nx, double *ny, double bx, double by )
{
   double cx,cy, gx,gy;

   /* Get parameters. */
   gl_cameraGet( &cx, &cy );
   gui_getOffset( &gx, &gy );

   /* calculate position - we'll use relative coords to player */
   *nx = (bx - cx + gx) * gl_cameraZ;
   *ny = (by - cy + gy) * gl_cameraZ;
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
   double x,y, w,h, tx,ty;

   /* Translate coords. */
   gl_gameToScreenCoords( &x, &y, bx - sprite->sw/2., by - sprite->sh/2. );

   /* Scaled sprite dimensions. */
   w = sprite->sw*gl_cameraZ;
   h = sprite->sh*gl_cameraZ;

   /* check if inbounds */
   if ((x < -w) || (x > SCREEN_W+w) ||
         (y > -h) || (y > SCREEN_H+h))
      return;

   /* texture coords */
   tx = sprite->sw*(double)(sx)/sprite->rw;
   ty = sprite->sh*(sprite->sy-(double)sy-1)/sprite->rh;

   gl_blitTexture( sprite, x, y, w, h,
         tx, ty, sprite->srw, sprite->srh, c );
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
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_blitSpriteInterpolateScale( const glTexture* sa, const glTexture *sb,
      double inter, const double bx, const double by,
      double scalew, double scaleh,
      const int sx, const int sy, const glColour *c )
{
   double x,y, w,h, tx,ty;

   /* Translate coords. */
   gl_gameToScreenCoords( &x, &y, bx - scalew * sa->sw/2., by - scaleh * sa->sh/2. );

   /* Scaled sprite dimensions. */
   w = sa->sw*gl_cameraZ*scalew;
   h = sa->sh*gl_cameraZ*scaleh;

   /* check if inbounds */
   if ((x < -w) || (x > SCREEN_W+w) ||
         (y > -h) || (y > SCREEN_H+h))
      return;

   /* texture coords */
   tx = sa->sw*(double)(sx)/sa->rw;
   ty = sa->sh*(sa->sy-(double)sy-1)/sa->rh;

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
   tx = sprite->sw*(double)(sx)/sprite->rw;
   ty = sprite->sh*(sprite->sy-(double)sy-1)/sprite->rh;

   /* actual blitting */
   gl_blitTexture( sprite, x, y, sprite->sw, sprite->sh,
         tx, ty, sprite->srw, sprite->srh, c );
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
   tx = sprite->sw*(double)(sx)/sprite->rw;
   ty = sprite->sh*(sprite->sy-(double)sy-1)/sprite->rh;

   /* actual blitting */
   gl_blitTexture( sprite, x, y, bw, bh,
         tx, ty, sprite->srw, sprite->srh, c );
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
         tx, ty, texture->srw, texture->srh, c );
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
         0., 0., texture->srw, texture->srh, c );
}


/**
 * @brief Binds the camera to a vector.
 *
 * All stuff displayed with relative functions will be affected by the camera's
 *  position.  Does not affect stuff in screen coordinates.
 *
 *    @param pos Vector to use as camera.
 */
void gl_cameraBind( Vector2d* pos )
{
   gl_camera = pos;
}


/**
 * @brief Makes the camera static and set on a position.
 *
 *    @param x X position to set camera to.
 *    @param y Y position to set camera to.
 */
void gl_cameraStatic( double x, double y )
{
   gl_cameraX = x;
   gl_cameraY = y;
   gl_camera  = NULL;
}


/**
 * @brief Gets the camera position.
 *
 *    @param[out] x X position to get.
 *    @param[out] y Y position to get.
 */
void gl_cameraGet( double *x, double *y )
{
   if (gl_camera != NULL) {
      *x = gl_camera->x;
      *y = gl_camera->y;
   }
   else {
      *x = gl_cameraX;
      *y = gl_cameraY;
   }
}


/**
 * @brief Draws an empty circle.
 *
 *    @param cx X position of the center in screen coordinates..
 *    @param cy Y position of the center in screen coordinates.
 *    @param r Radius of the circle.
 *    @param c Colour to use.
 */
#define PIXEL(x,y)   \
if (i<OPENGL_RENDER_VBO_SIZE) { \
   vertex[2*i+0] = x; \
   vertex[2*i+1] = y; \
   i++; \
}
static void gl_drawCircleEmpty( const double cx, const double cy,
      const double r, const glColour *c )
{
   int i, j;
   double x,y,p;
   GLfloat vertex[2*OPENGL_RENDER_VBO_SIZE], col[4*OPENGL_RENDER_VBO_SIZE];

   /* Starting parameters. */
   i = 0;
   x = 0;
   y = r;
   p = (5. - (r*4.)) / 4.;

   PIXEL( cx,   cy+y );
   PIXEL( cx,   cy-y );
   PIXEL( cx+y, cy   );
   PIXEL( cx-y, cy   );

   while (x<y) {
      x++;
      if (p < 0) p += 2*(double)(x)+1;
      else p += 2*(double)(x-(--y))+1;

      if (x==0) {
         PIXEL( cx,   cy+y );
         PIXEL( cx,   cy-y );
         PIXEL( cx+y, cy   );
         PIXEL( cx-y, cy   );
      }
      else
         if (x==y) {
            PIXEL( cx+x, cy+y );
            PIXEL( cx-x, cy+y );
            PIXEL( cx+x, cy-y );
            PIXEL( cx-x, cy-y );
         }
         else
            if (x<y) {
               PIXEL( cx+x, cy+y );
               PIXEL( cx-x, cy+y );
               PIXEL( cx+x, cy-y );
               PIXEL( cx-x, cy-y );
               PIXEL( cx+y, cy+x );
               PIXEL( cx-y, cy+x );
               PIXEL( cx+y, cy-x );
               PIXEL( cx-y, cy-x );
            }
   }
   gl_vboSubData( gl_renderVBO, 0, i*2*sizeof(GLfloat), vertex );
   gl_vboActivateOffset( gl_renderVBO, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );

   /* Set up the colour. */
   for (j=0; j<i; j++) {
      col[4*j+0] = c->r;
      col[4*j+1] = c->g;
      col[4*j+2] = c->b;
      col[4*j+3] = c->a;
   }
   gl_vboSubData( gl_renderVBO, gl_renderVBOcolOffset, j*4*sizeof(GLfloat), col );
   gl_vboActivateOffset( gl_renderVBO, GL_COLOR_ARRAY,
         gl_renderVBOcolOffset, 4, GL_FLOAT, 0 );

   /* Draw. */
   glDrawArrays( GL_POINTS, 0, i );

   /* Clear state. */
   gl_vboDeactivate();
}
#undef PIXEL


/**
 * @brief Draws a circle.
 *
 *    @param cx X position of the center in screen coordinates..
 *    @param cy Y position of the center in screen coordinates.
 *    @param r Radius of the circle.
 *    @param c Colour to use.
 *    @param filled yWhether or not it should be filled.
 */
void gl_drawCircle( const double cx, const double cy,
      const double r, const glColour *c, int filled )
{
   if (filled)
      gl_blitTexture( gl_circle, cx-r, cy-r, 2.*r, 2.*r,
         0., 0., gl_circle->srw, gl_circle->srh, c );
   else
      gl_drawCircleEmpty( cx, cy, r, c );
}


/**
 * @brief Only displays the pixel if it's in the screen.
 */
#define PIXEL(x,y)   \
if ((x>rx) && (y>ry) && (x<rxw) && (y<ryh) && (i<OPENGL_RENDER_VBO_SIZE)) { \
   vertex[2*i+0] = x; \
   vertex[2*i+1] = y; \
   i++; \
}
/**
 * @brief Draws a circle in a rectangle.
 *
 *    @param cx X position of the center in screen coordinates..
 *    @param cy Y position of the center in screen coordinates.
 *    @param r Radius of the circle.
 *    @param rx X position of the rectangle limiting the circle in screen coords.
 *    @param ry Y position of the rectangle limiting the circle in screen coords.
 *    @param rw Width of the limiting rectangle.
 *    @param rh Height of the limiting rectangle.
 *    @param c Colour to use.
 */
void gl_drawCircleInRect( const double cx, const double cy, const double r,
      const double rx, const double ry, const double rw, const double rh,
      const glColour *c, int filled )
{
   int i, j;
   double rxw,ryh, x,y,p, w,h;
   GLfloat vertex[2*OPENGL_RENDER_VBO_SIZE], col[4*OPENGL_RENDER_VBO_SIZE];

   rxw = rx+rw;
   ryh = ry+rh;

   /* is offscreen? */
   if ((cx+r < rx) || (cy+r < ry) || (cx-r > rxw) || (cy-r > ryh))
      return;
   /* can be drawn normally? */
   else if ((cx-r > rx) && (cy-r > ry) && (cx+r < rxw) && (cy+r < ryh)) {
      gl_drawCircle( cx, cy, r, c, filled );
      return;
   }

   /* Case if filled. */
   if (filled) {
      x = CLAMP( rx, rxw, cx-r );
      y = CLAMP( ry, ryh, cy-r );
      w = CLAMP( 0., rxw-x,  2.*r );
      h = CLAMP( 0., ryh-y,  2.*r );
      gl_blitTexture( gl_circle, x, y, w, h,
            (x-(cx-r))/(2.*r) * gl_circle->srw,
            (y-(cy-r))/(2.*r) * gl_circle->srh,
            (w/(2.*r)) * gl_circle->srw,
            (h/(2.*r)) * gl_circle->srh, c );
      return;
   }

   /* Starting parameters. */
   i = 0;
   x = 0;
   y = r;
   p = (5. - (r*4.)) / 4.;

   PIXEL( cx,   cy+y );
   PIXEL( cx,   cy-y );
   PIXEL( cx+y, cy   );
   PIXEL( cx-y, cy   );

   while (x<y) {
      x++;
      if (p < 0) p += 2*(double)(x)+1;
      else p += 2*(double)(x-(--y))+1;

      if (x==0) {
         PIXEL( cx,   cy+y );
         PIXEL( cx,   cy-y );
         PIXEL( cx+y, cy   );
         PIXEL( cx-y, cy   );
      }
      else
         if (x==y) {
            PIXEL( cx+x, cy+y );
            PIXEL( cx-x, cy+y );
            PIXEL( cx+x, cy-y );
            PIXEL( cx-x, cy-y );
         }
         else
            if (x<y) {
               PIXEL( cx+x, cy+y );
               PIXEL( cx-x, cy+y );
               PIXEL( cx+x, cy-y );
               PIXEL( cx-x, cy-y );
               PIXEL( cx+y, cy+x );
               PIXEL( cx-y, cy+x );
               PIXEL( cx+y, cy-x );
               PIXEL( cx-y, cy-x );
            }
   }
   gl_vboSubData( gl_renderVBO, 0, i*2*sizeof(GLfloat), vertex );
   gl_vboActivateOffset( gl_renderVBO, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );

   /* Set up the colour. */
   for (j=0; j<i; j++) {
      col[4*j+0] = c->r;
      col[4*j+1] = c->g;
      col[4*j+2] = c->b;
      col[4*j+3] = c->a;
   }
   gl_vboSubData( gl_renderVBO, gl_renderVBOcolOffset, i*4*sizeof(GLfloat), col );
   gl_vboActivateOffset( gl_renderVBO, GL_COLOR_ARRAY,
         gl_renderVBOcolOffset, 4, GL_FLOAT, 0 );

   /* Draw. */
   glDrawArrays( GL_POINTS, 0, i );

   /* Clear state. */
   gl_vboDeactivate();
}
#undef PIXEL



/**
 * @brief Generates an filled circle texture.
 *
 *    @param radius Radius of the circle to generate.
 *    @return The tetxure containing the generated circle.
 */
static glTexture *gl_genCircle( int radius )
{
   int i,j,k, n,m;
   SDL_Surface *sur;
   uint8_t *pix, *buf;
   int h, w;
   double a;

   /* Calculate parameters. */
   w = 2*radius+1;
   h = 2*radius+1;

   /* Create the surface. */
   sur = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, 32, RGBAMASK );
   pix = sur->pixels;

   /* Generate the circle. */
   SDL_LockSurface( sur );

   /* Create temporary buffer to draw circle in. */
   k = 3;
   buf = malloc( (h*k) * (w*k) );
   for (i=0; i<k*h; i++) {
      for (j=0; j<k*w; j++) {
         if (pow2(i-k*radius)+pow2(j-k*radius) < pow2(k*radius))
            buf[ i*k*w + j] = 0xFF;
      }
   }

   /* Draw the circle with filter. */
   for (i=0; i<h; i++) {
      for (j=0; j<w; j++) {
         /* Calculate blur. */
         a = 0.;
         for (n=0; n<k; n++) {
            for (m=0; m<k; m++) {
               a += buf[ (i*k+n)*k*w + (j*k+m) ];
            }
         }
         a /= k*k;

         /* Set pixel. */
         pix[i*sur->pitch + j*4 + 0] = 0xFF;
         pix[i*sur->pitch + j*4 + 1] = 0xFF;
         pix[i*sur->pitch + j*4 + 2] = 0xFF;
         pix[i*sur->pitch + j*4 + 3] = (uint8_t)a;
      }
   }

   /* CLean up. */
   free(buf);

   SDL_UnlockSurface( sur );

   /* Return texture. */
   return gl_loadImage( sur, OPENGL_TEX_MIPMAPS );
}


/**
 * @brief Initializes the OpenGL rendering routines.
 *
 *    @return 0 on success.
 */
int gl_initRender (void)
{
   /* Initialize the VBO. */
   gl_renderVBO = gl_vboCreateStream( sizeof(GLfloat) *
         OPENGL_RENDER_VBO_SIZE*(2 + 2 + 4), NULL );
   gl_renderVBOtexOffset = sizeof(GLfloat) * OPENGL_RENDER_VBO_SIZE*2;
   gl_renderVBOcolOffset = sizeof(GLfloat) * OPENGL_RENDER_VBO_SIZE*(2+2);

   /* Initialize the circles. */
   gl_circle      = gl_genCircle( 128 );

   return 0;
}


/**
 * @brief Cleans up the OpenGL rendering routines.
 */
void gl_exitRender (void)
{
   /* Destroy the VBO. */
   gl_vboDestroy( gl_renderVBO );
   gl_renderVBO = NULL;

   /* Destroy the circles. */
   gl_freeTexture(gl_circle);
   gl_circle = NULL;
}

