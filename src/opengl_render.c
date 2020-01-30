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


#include "opengl.h"

#include "naev.h"

#include "log.h"
#include "ndata.h"
#include "gui.h"
#include "conf.h"
#include "camera.h"
#include "nstring.h"


#define OPENGL_RENDER_VBO_SIZE      256 /**< Size of VBO. */


static gl_vbo *gl_renderVBO = 0; /**< VBO for rendering stuff. */
static gl_vbo *gl_squareVBO = 0;
static gl_vbo *gl_squareEmptyVBO = 0;
static int gl_renderVBOtexOffset = 0; /**< VBO texture offset. */
static int gl_renderVBOcolOffset = 0; /**< VBO colour offset. */

/* GLSL programs, uniform indexes, attribute indexes */
static GLuint texture_glsl_program = 0;
static GLuint texture_glsl_program_vertex = 0;
static GLuint texture_glsl_program_projection = 0;
static GLuint texture_glsl_program_color = 0;
static GLuint texture_glsl_program_tex_mat = 0;
static GLuint texture_interpolate_glsl_program = 0;
static GLuint texture_interpolate_glsl_program_vertex = 0;
static GLuint texture_interpolate_glsl_program_projection = 0;
static GLuint texture_interpolate_glsl_program_color = 0;
static GLuint texture_interpolate_glsl_program_tex_mat = 0;
static GLuint texture_interpolate_glsl_program_sampler1 = 0;
static GLuint texture_interpolate_glsl_program_sampler2 = 0;
static GLuint texture_interpolate_glsl_program_inter = 0;
static GLuint rect_glsl_program = 0;
static GLuint rect_glsl_program_color = 0;
static GLuint rect_glsl_program_projection = 0;
static GLuint rect_glsl_program_vertex = 0;


/*
 * Circle textures.
 */
static glTexture *gl_circle      = NULL; /**< Circle mipmap. */


/*
 * prototypes
 */
static void gl_drawCircleEmpty( const double cx, const double cy,
      const double r, const glColour *c );
static void gl_blitTextureInterpolate(  const glTexture* ta,
      const glTexture* tb, const double inter,
      const double x, const double y,
      const double w, const double h,
      const double tx, const double ty,
      const double tw, const double th, const glColour *c );
static GLuint texture_glsl_program_compile( void );


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

   glUseProgram(rect_glsl_program);

   /* Set the vertex. */
   projection = gl_view_matrix;
   projection = gl_Matrix4_Translate(projection, x, y, 0);
   projection = gl_Matrix4_Scale(projection, w, h, 1);
   glEnableVertexAttribArray( texture_glsl_program_vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, texture_glsl_program_vertex,
         0, 2, GL_FLOAT, 0 );

   /* Set shader uniforms. */
   glUniform4f(rect_glsl_program_color, c->r, c->g, c->b, c->a);
   gl_Matrix4_Uniform(rect_glsl_program_projection, projection);

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   gl_vboDeactivate();
   glDisableVertexAttribArray( texture_glsl_program_vertex );
   glUseProgram(0);

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
   gl_Matrix4 projection;

   glUseProgram(rect_glsl_program);

   /* Set the vertex. */
   projection = gl_view_matrix;
   projection = gl_Matrix4_Translate(projection, x, y, 0);
   projection = gl_Matrix4_Scale(projection, w, h, 1);
   glEnableVertexAttribArray( texture_glsl_program_vertex );
   gl_vboActivateAttribOffset( gl_squareEmptyVBO, texture_glsl_program_vertex,
         0, 2, GL_FLOAT, 0 );

   /* Set shader uniforms. */
   glUniform4f(rect_glsl_program_color, c->r, c->g, c->b, c->a);
   gl_Matrix4_Uniform(rect_glsl_program_projection, projection);

   /* Draw. */
   glDrawArrays( GL_LINE_STRIP, 0, 5 );

   /* Clear state. */
   gl_vboDeactivate();
   glDisableVertexAttribArray( texture_glsl_program_vertex );
   glUseProgram(0);

   /* Check errors. */
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
   int i;
   GLfloat vertex[2*4], colours[4*4];
   GLfloat vx,vy, vr;

   /* Set up stuff. */
   vx = x;
   vy = y;
   vr = r;

   /* the + sign in the middle of the radar representing the player */
   for (i=0; i<4; i++) {
      colours[4*i + 0] = c->r;
      colours[4*i + 1] = c->g;
      colours[4*i + 2] = c->b;
      colours[4*i + 3] = c->a;
   }
   gl_vboSubData( gl_renderVBO, gl_renderVBOcolOffset,
         sizeof(GLfloat) * 4*4, colours );
   /* Set up vertex. */
   vertex[0] = vx+0.;
   vertex[1] = vy-vr;
   vertex[2] = vx+0.;
   vertex[3] = vy+vr;
   vertex[4] = vx-vr;
   vertex[5] = vy+0.;
   vertex[6] = vx+vr;
   vertex[7] = vy+0.;
   gl_vboSubData( gl_renderVBO, 0, sizeof(GLfloat) * 4*2, vertex );
   /* Draw tho VBO. */
   gl_vboActivateOffset( gl_renderVBO, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
   gl_vboActivateOffset( gl_renderVBO, GL_COLOR_ARRAY,
         gl_renderVBOcolOffset, 4, GL_FLOAT, 0 );
   glDrawArrays( GL_LINES, 0, 4 );
   gl_vboDeactivate();
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
 */
void gl_blitTexture(  const glTexture* texture,
      const double x, const double y,
      const double w, const double h,
      const double tx, const double ty,
      const double tw, const double th, const glColour *c )
{
   gl_Matrix4 projection, tex_mat;

   if (texture_glsl_program == 0) {
      return;
   }

   glUseProgram(texture_glsl_program);

   /* Bind the texture. */
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, texture->texture);

   /* Must have colour for now. */
   if (c == NULL)
      c = &cWhite;

   /* Set the vertex. */
   projection = gl_view_matrix;
   projection = gl_Matrix4_Translate(projection, x, y, 0);
   projection = gl_Matrix4_Scale(projection, w, h, 1);
   glEnableVertexAttribArray( texture_glsl_program_vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, texture_glsl_program_vertex,
         0, 2, GL_FLOAT, 0 );

   /* Set the texture. */
   tex_mat = gl_Matrix4_Identity();
   tex_mat = gl_Matrix4_Translate(tex_mat, tx, ty, 0);
   tex_mat = gl_Matrix4_Scale(tex_mat, tw, th, 1);

   /* Set shader uniforms. */
   glUniform4f(texture_glsl_program_color, c->r, c->g, c->b, c->a);
   gl_Matrix4_Uniform(texture_glsl_program_projection, projection);
   gl_Matrix4_Uniform(texture_glsl_program_tex_mat, tex_mat);

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   gl_vboDeactivate();
   glDisable(GL_TEXTURE_2D);
   glDisableVertexAttribArray( glGetAttribLocation(texture_glsl_program, "vertex") );

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
      return;
   }

   gl_Matrix4 projection, tex_mat;

   if (texture_interpolate_glsl_program == 0) {
      return;
   }

   glUseProgram(texture_interpolate_glsl_program);

   /* Bind the textures. */
   nglActiveTexture( GL_TEXTURE0 );
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, ta->texture);
   nglActiveTexture( GL_TEXTURE1 );
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, tb->texture);

   /* Must have colour for now. */
   if (c == NULL)
      c = &cWhite;

   /* Set the vertex. */
   projection = gl_view_matrix;
   projection = gl_Matrix4_Translate(projection, x, y, 0);
   projection = gl_Matrix4_Scale(projection, w, h, 1);
   gl_vboActivateOffset( gl_squareVBO, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );

   /* Set the texture. */
   tex_mat = gl_Matrix4_Identity();
   tex_mat = gl_Matrix4_Translate(tex_mat, tx, ty, 0);
   tex_mat = gl_Matrix4_Scale(tex_mat, tw, th, 1);

   /* Set shader uniforms. */
   glUniform1i(texture_interpolate_glsl_program_sampler1, 0);
   glUniform1i(texture_interpolate_glsl_program_sampler2, 1);
   glUniform4f(texture_interpolate_glsl_program_color, c->r, c->g, c->b, c->a);
   glUniform1f(texture_interpolate_glsl_program_inter, inter);
   gl_Matrix4_Uniform(texture_interpolate_glsl_program_projection, projection);
   gl_Matrix4_Uniform(texture_interpolate_glsl_program_tex_mat, tex_mat);

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   gl_vboDeactivate();
   glDisable(GL_TEXTURE_2D);
   nglActiveTexture( GL_TEXTURE0 );
   glDisable(GL_TEXTURE_2D);

   /* anything failed? */
   gl_checkErr();

   glUseProgram(0);
}


/**
 * @brief Converts ingame coordinates to screen coordinates.
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
 * @brief Converts screen coordinates to ingame coordinates.
 *
 *    @param[out] nx New ingame X coord.
 *    @param[out] ny New ingame Y coord.
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


void gl_drawCircleLoop( const double cx, const double cy,
      const double r, const glColour *c )
{
   int i, points;
   double angi, cosi, sini;
   double nxc, xc, yc;
   GLfloat vertex[2*OPENGL_RENDER_VBO_SIZE], col[4*OPENGL_RENDER_VBO_SIZE];

   /* Aim for 10 px between each vertex. */
   points = CLAMP( 8, OPENGL_RENDER_VBO_SIZE, (int)ceil(M_PI * r * 5.) );

   angi = 2. * M_PI / (double)points;
   cosi = cos(angi);
   sini = sin(angi);

   vertex[0] = cx + r;
   vertex[1] = cy;

   xc = 1.;
   yc = 0.;

   /* Calculate the vertices by iterating counter-clockwise. */
   for (i=1; i<points; i++) {
      nxc = cosi * xc - sini * yc;
      yc  = sini * xc + cosi * yc;
      xc  = nxc;

      vertex[i*2+0] = cx + xc * r;
      vertex[i*2+1] = cy + yc * r;
   }

   gl_vboSubData( gl_renderVBO, 0, points*2*sizeof(GLfloat), vertex );
   gl_vboActivateOffset( gl_renderVBO, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );

   /* Set up the colour. */
   for (i=0; i<points; i++) {
      col[4*i+0] = c->r;
      col[4*i+1] = c->g;
      col[4*i+2] = c->b;
      col[4*i+3] = c->a;
   }

   gl_vboSubData( gl_renderVBO, gl_renderVBOcolOffset, points*4*sizeof(GLfloat), col );
   gl_vboActivateOffset( gl_renderVBO, GL_COLOR_ARRAY,
         gl_renderVBOcolOffset, 4, GL_FLOAT, 0 );

   /* Draw. */
   glDrawArrays( GL_LINE_LOOP, 0, points );

   /* Clear state. */
   gl_vboDeactivate();
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
 *    @param filled Whether or not it should be filled.
 */
void gl_drawCircle( const double cx, const double cy,
      const double r, const glColour *c, int filled )
{
   if (filled)
      gl_blitTexture( gl_circle, cx-r, cy-r, 2.*r, 2.*r,
         0., 0., gl_circle->srw, gl_circle->srh, c );
   else if (gl_vendorIsIntel())
      gl_drawCircleEmpty( cx, cy, r, c );
   else
      gl_drawCircleLoop( cx, cy, r, c );
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
   double rxw,ryh, x,y,p, w,h, tx,ty, tw,th, r2;
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
      r2 = 2.*r;
      /* Clamp bottom left. */
      x  = CLAMP( rx, rxw, cx-r );
      y  = CLAMP( ry, ryh, cy-r );
      /* Clamp width. */
      w  = CLAMP( rx, rxw, cx+r ) - x;
      h  = CLAMP( ry, ryh, cy+r ) - y;
      /* Calculate texture bottom left. */
      tx  = x - (cx-r);
      tx *= gl_circle->srw / r2; /* Transform to unitary coordinates. */
      ty  = y - (cy-r);
      ty *= gl_circle->srh / r2;
      /* Calculate dimensions of texture. */
      tw  = w/r2 * gl_circle->srw;
      th  = h/r2 * gl_circle->srh;
      /* Render. */
      gl_blitTexture( gl_circle, x, y, w, h, tx, ty, tw, th, c );
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
glTexture *gl_genCircle( int radius )
{
   int i, j, edge, blur;
   SDL_Surface *sur;
   uint8_t *pix;
   int h, w;
   double dist;
   char name[PATH_MAX];

   /* Calculate parameters. */
   w = 2*radius+1;
   h = 2*radius+1;

   /* Create the surface. */
   sur = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, 32, RGBAMASK );
   pix = sur->pixels;

   /* Generate the circle. */
   SDL_LockSurface( sur );

   edge = pow2(radius);
   blur = pow2(radius - 1);

   for (i=0; i<w; i++) {
      for (j=0; j<h; j++) {
         dist = pow2(i - radius) + pow2(j - radius);
         if (dist > edge)
            pix[i*sur->pitch + j*4 + 3] = 0;
         else if (dist <= blur)
            pix[i*sur->pitch + j*4 + 3] = 0xFF;
         else
            pix[i*sur->pitch + j*4 + 3] = CLAMP(0, 0xFF, (edge - dist) / (edge - blur) * 0xFF);

         pix[i*sur->pitch + j*4 + 0] = 0xFF;
         pix[i*sur->pitch + j*4 + 1] = 0xFF;
         pix[i*sur->pitch + j*4 + 2] = 0xFF;
      }
   }

   SDL_UnlockSurface( sur );

   /* Return texture. */
   nsnprintf( name, sizeof(name), "gencircle%d", radius );
   return gl_loadImagePad( name, sur, OPENGL_TEX_MIPMAPS, sur->w, sur->h, 1, 1, 1 );
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

   /* Initialize the circles. */
   gl_circle      = gl_genCircle( 128 );

   vertex[0] = 0;
   vertex[1] = 0;
   vertex[2] = 1;
   vertex[3] = 0;
   vertex[4] = 0;
   vertex[5] = 1;
   vertex[6] = 1;
   vertex[7] = 1;
   gl_squareVBO = gl_vboCreateStatic( sizeof(GLfloat) * 8, vertex );

   vertex[0] = 0;
   vertex[1] = 0;
   vertex[2] = 1;
   vertex[3] = 0;
   vertex[4] = 1;
   vertex[5] = 1;
   vertex[6] = 0;
   vertex[7] = 1;
   vertex[8] = 0;
   vertex[9] = 0;
   gl_squareEmptyVBO = gl_vboCreateStatic( sizeof(GLfloat) * 8, vertex );

   texture_glsl_program = gl_program_vert_frag("texture.vert", "texture.frag");
   texture_glsl_program_vertex = glGetAttribLocation(texture_glsl_program, "vertex");
   texture_glsl_program_projection = glGetUniformLocation(texture_glsl_program, "projection");
   texture_glsl_program_color = glGetUniformLocation(texture_glsl_program, "color");
   texture_glsl_program_tex_mat = glGetUniformLocation(texture_glsl_program, "tex_mat");

   texture_interpolate_glsl_program = gl_program_vert_frag("texture.vert",
         "texture_interpolate.frag");
   texture_interpolate_glsl_program_projection = glGetUniformLocation(texture_interpolate_glsl_program, "projection");
   texture_interpolate_glsl_program_color = glGetUniformLocation(texture_interpolate_glsl_program, "color");
   texture_interpolate_glsl_program_tex_mat = glGetUniformLocation(texture_interpolate_glsl_program, "tex_mat");
   texture_interpolate_glsl_program_sampler1 = glGetUniformLocation(texture_interpolate_glsl_program, "sampler1");
   texture_interpolate_glsl_program_sampler2 = glGetUniformLocation(texture_interpolate_glsl_program, "sampler2");
   texture_interpolate_glsl_program_inter = glGetUniformLocation(texture_interpolate_glsl_program, "inter");

   rect_glsl_program = gl_program_vert_frag("rect.vert", "rect.frag");
   rect_glsl_program_projection = glGetUniformLocation(rect_glsl_program, "projection");
   rect_glsl_program_color = glGetUniformLocation(rect_glsl_program, "color");
   rect_glsl_program_vertex = glGetAttribLocation(rect_glsl_program, "vertex");

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
   gl_vboDestroy( gl_squareEmptyVBO );
   gl_renderVBO = NULL;

   /* Destroy the circles. */
   gl_freeTexture(gl_circle);
   gl_circle = NULL;
}
