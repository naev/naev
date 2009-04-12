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


static Vector2d* gl_camera  = NULL; /**< Camera we are using. */
static double gl_cameraX    = 0.; /**< X position of camera. */
static double gl_cameraY    = 0.; /**< Y position of camera. */


/*
 * prototypes
 */
static void (*gl_blitTexture)( const glTexture* texture, 
      const double x, const double y,
      const double w, const double h,
      const double tx, const double ty, const glColour *c );
static void gl_blitTextureVertexArray(  const glTexture* texture,
      const double x, const double y,
      const double w, const double h,
      const double tx, const double ty, const glColour *c );


/**
 * @brief Texture array backend of gl_blitTexture.
 *
 *    @param texture Texture to blit.
 *    @param x X position of the texture on the screen.
 *    @param y Y position of the texture on the screen.
 *    @param tx X position within the texture.
 *    @param ty Y position within the texture.
 *    @param c Colour to use (modifies texture colour).
 */
static void gl_blitTextureVertexArray(  const glTexture* texture,
      const double x, const double y,
      const double w, const double h,
      const double tx, const double ty, const glColour *c )
{
   double tw,th;
   GLfloat vertex[4*2], tex[4*2];

   /* texture dimensions */
   tw = texture->sw / texture->rw;
   th = texture->sh / texture->rh;

   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, texture->texture);

   if (c==NULL)
      glColor4d( 1., 1., 1., 1. );
   else COLOUR(*c);

   /* Set the vertex. */
   glEnableClientState(GL_VERTEX_ARRAY);
   vertex[0] = (GLfloat)x;
   vertex[4] = vertex[0];
   vertex[2] = vertex[0] + (GLfloat)w;
   vertex[6] = vertex[2];
   vertex[1] = (GLfloat)y;
   vertex[3] = vertex[1];
   vertex[5] = vertex[1] + (GLfloat)h;
   vertex[7] = vertex[5];
   glVertexPointer( 2, GL_FLOAT, 0, vertex );

   /* Set the texture. */
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   tex[0] = (GLfloat)tx;
   tex[4] = tex[0];
   tex[2] = tex[0] + (GLfloat)tw;
   tex[6] = tex[2];
   tex[1] = (GLfloat)ty;
   tex[3] = tex[1];
   tex[5] = tex[1] + (GLfloat)th;
   tex[7] = tex[5];
   glTexCoordPointer( 2, GL_FLOAT, 0, tex );

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);

   glDisable(GL_TEXTURE_2D);

   /* anything failed? */
   gl_checkErr();
}


/**
 * @brief Blits a sprite, position is relative to the player.
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
   double x,y, tx,ty, cx,cy, gx,gy;

   /* Get parameters. */
   gl_cameraGet( &cx, &cy );
   gui_getOffset( &gx, &gy );

   /* calculate position - we'll use relative coords to player */
   x = bx - cx - sprite->sw/2. + gx;
   y = by - cy - sprite->sh/2. + gy;

   /* check if inbounds */
   if ((fabs(x) > SCREEN_W/2 + sprite->sw) ||
         (fabs(y) > SCREEN_H/2 + sprite->sh) )
      return;

   /* texture coords */
   tx = sprite->sw*(double)(sx)/sprite->rw;
   ty = sprite->sh*(sprite->sy-(double)sy-1)/sprite->rh;

   gl_blitTexture( sprite, x, y, sprite->sw, sprite->sh, tx, ty, c );
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

   x = bx - (double)SCREEN_W/2.;
   y = by - (double)SCREEN_H/2.;

   /* texture coords */
   tx = sprite->sw*(double)(sx)/sprite->rw;
   ty = sprite->sh*(sprite->sy-(double)sy-1)/sprite->rh;

   /* actual blitting */
   gl_blitTexture( sprite, x, y, sprite->sw, sprite->sh, tx, ty, c );
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
   x = bx - (double)SCREEN_W/2.;
   y = by - (double)SCREEN_H/2.;

   /* texture dimensions */
   tx = ty = 0.;

   /* Actual blitting. */
   gl_blitTexture( texture, x, y, bw, bh, tx, ty, c );
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
   x = bx - (double)SCREEN_W/2.;
   y = by - (double)SCREEN_H/2.;

   /* actual blitting */
   gl_blitTexture( texture, x, y, texture->sw, texture->sh, 0, 0, c );
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
 * @brief Draws a circle.
 *
 *    @param cx X position of the center in screen coordinates..
 *    @param cy Y position of the center in screen coordinates.
 *    @param r Radius of the circle.
 */
void gl_drawCircle( const double cx, const double cy, const double r )
{
   double x,y,p;

   x = 0;
   y = r;
   p = (5. - (r*4.)) / 4.;

   glBegin(GL_POINTS);
      glVertex2d( cx,   cy+y );
      glVertex2d( cx,   cy-y );
      glVertex2d( cx+y, cy   );
      glVertex2d( cx-y, cy   );

      while (x<y) {
         x++;
         if (p < 0) p += 2*(double)(x)+1;
         else p += 2*(double)(x-(--y))+1;

         if (x==0) {
            glVertex2d( cx,   cy+y );
            glVertex2d( cx,   cy-y );
            glVertex2d( cx+y, cy   );
            glVertex2d( cx-y, cy   );
         }
         else
            if (x==y) {
               glVertex2d( cx+x, cy+y );
               glVertex2d( cx-x, cy+y );
               glVertex2d( cx+x, cy-y );
               glVertex2d( cx-x, cy-y );
            }
            else
               if (x<y) {
                  glVertex2d( cx+x, cy+y );
                  glVertex2d( cx-x, cy+y );
                  glVertex2d( cx+x, cy-y );
                  glVertex2d( cx-x, cy-y );
                  glVertex2d( cx+y, cy+x );
                  glVertex2d( cx-y, cy+x );
                  glVertex2d( cx+y, cy-x );
                  glVertex2d( cx-y, cy-x );
               }
      }
   glEnd(); /* GL_POINTS */

   gl_checkErr();
}


/**
 * @brief Only displays the pixel if it's in the screen.
 */
#define PIXEL(x,y)   \
if ((x>rx) && (y>ry) && (x<rxw) && (y<ryh))  \
   glVertex2d(x,y)
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
 */
void gl_drawCircleInRect( const double cx, const double cy, const double r,
      const double rx, const double ry, const double rw, const double rh )
{
   double rxw,ryh, x,y,p;

   rxw = rx+rw;
   ryh = ry+rh;

   /* is offscreen? */
   if ((cx+r < rx) || (cy+r < ry) || (cx-r > rxw) || (cy-r > ryh))
      return;
   /* can be drawn normally? */
   else if ((cx-r > rx) && (cy-r > ry) && (cx+r < rxw) && (cy+r < ryh)) {
      gl_drawCircle( cx, cy, r );
      return;
   }

   x = 0;
   y = r;    
   p = (5. - (r*4.)) / 4.;

   glBegin(GL_POINTS);
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
   glEnd(); /* GL_POINTS */

   gl_checkErr();
}
#undef PIXEL


/**
 * @brief Initializes the OpenGL rendering routines.
 *
 *    @return 0 on success.
 */
int gl_initRender (void)
{
   gl_blitTexture = gl_blitTextureVertexArray;
   return 0;
}


/**
 * @brief Cleans up the OpenGL rendering routines.
 */
void gl_exitRender (void)
{
   gl_blitTexture = NULL;
}

