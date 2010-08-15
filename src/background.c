/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file background.c
 *
 * @brief Handles displaying backgrounds.
 */

#include "background.h"

#include "naev.h"

#include "nxml.h"

#include "opengl.h"
#include "log.h"
#include "player.h"
#include "conf.h"
#include "rng.h"
#include "pause.h"
#include "array.h"
#include "ndata.h"
#include "nlua.h"
#include "nluadef.h"
#include "nlua_tex.h"
#include "nlua_col.h"
#include "nlua_bkg.h"


/**
 * @brief Represents a background image like say a Nebula.
 */
typedef struct background_image_s {
   glTexture *image; /**< Image to display. */
   double x; /**< X center of the image. */
   double y; /**< Y center of the image. */
   double move; /**< How many pixels it moves for each pixel the player moves. */
   double scale; /**< How the image should be scaled. */
   glColour col; /**< Colour to use. */
} background_image_t;
static background_image_t *bkg_image_arr = NULL; /**< Background image array to display. */


/**
 * @brief Backgrounds.
 */
static lua_State *bkg_cur_L = NULL; /**< Current Lua state. */
static lua_State *bkg_def_L = NULL; /**< Default Lua state. */


/*
 * Background stars.
 */
#define STAR_BUF     250 /**< Area to leave around screen for stars, more = less repetition */
static gl_vbo *star_vertexVBO = NULL; /**< Star Vertex VBO. */
static gl_vbo *star_colourVBO = NULL; /**< Star Colour VBO. */
static GLfloat *star_vertex = NULL; /**< Vertex of the stars. */
static GLfloat *star_colour = NULL; /**< Brightness of the stars. */
static unsigned int nstars = 0; /**< Total stars. */
static unsigned int mstars = 0; /**< Memory stars are taking. */


/*
 * Prototypes.
 */
static void background_renderImages (void);
static lua_State* background_create( const char *path );
static void background_clearCurrent (void);


/**
 * @brief Initilaizes background stars.
 *
 *    @param n Number of stars to add (stars per 800x640 screen).
 */
void background_initStars( int n )
{
   unsigned int i;
   GLfloat w, h, hw, hh;
   double size;

   /* Calculate size. */
   size  = SCREEN_W*SCREEN_H+STAR_BUF*STAR_BUF;
   size /= pow2(conf.zoom_far);

   /* Calculate star buffer. */
   w  = (SCREEN_W + 2.*STAR_BUF);
   w += conf.zoom_stars * (w / conf.zoom_far - 1.);
   h  = (SCREEN_H + 2.*STAR_BUF);
   h += conf.zoom_stars * (h / conf.zoom_far - 1.);
   hw = w / 2.;
   hh = h / 2.;

   /* Calculate stars. */
   size  *= n;
   nstars = (unsigned int)(size/(800.*600.));

   if (mstars < nstars) {
      /* Create data. */
      star_vertex = realloc( star_vertex, nstars * sizeof(GLfloat) * 4 );
      star_colour = realloc( star_colour, nstars * sizeof(GLfloat) * 8 );
      mstars = nstars;
   }
   for (i=0; i < nstars; i++) {
      /* Set the position. */
      star_vertex[4*i+0] = RNGF()*w - hw;
      star_vertex[4*i+1] = RNGF()*h - hh;
      star_vertex[4*i+2] = 0.;
      star_vertex[4*i+3] = 0.;
      /* Set the colour. */
      star_colour[8*i+0] = 1.;
      star_colour[8*i+1] = 1.;
      star_colour[8*i+2] = 1.;
      star_colour[8*i+3] = RNGF()*0.6 + 0.2;
      star_colour[8*i+4] = 1.;
      star_colour[8*i+5] = 1.;
      star_colour[8*i+6] = 1.;
      star_colour[8*i+7] = 0.;
   }

   /* Destroy old VBO. */
   if (star_vertexVBO != NULL) {
      gl_vboDestroy( star_vertexVBO );
      star_vertexVBO = NULL;
   }
   if (star_colourVBO != NULL) {
      gl_vboDestroy( star_colourVBO );
      star_colourVBO = NULL;
   }

   /* Create now VBO. */
   star_vertexVBO = gl_vboCreateStream(
         nstars * sizeof(GLfloat) * 4, star_vertex );
   star_colourVBO = gl_vboCreateStatic(
         nstars * sizeof(GLfloat) * 8, star_colour );
}




/**
 * @brief Renders the starry background.
 *
 *    @param dt Current delta tick.
 */
void background_renderStars( const double dt )
{
   unsigned int i;
   GLfloat hh, hw, h, w;
   GLfloat x, y, m, b;
   GLfloat brightness;
   double z;

   /*
    * gprof claims it's the slowest thing in the game!
    */

   /* Do some scaling for now. */
   gl_cameraZoomGet( &z );
   z = 1. * (1. - conf.zoom_stars) + z * conf.zoom_stars;
   gl_matrixPush();
      gl_matrixTranslate( SCREEN_W/2., SCREEN_H/2. );
      gl_matrixScale( z, z );

      if (!paused && (player.p != NULL) && !player_isFlag(PLAYER_DESTROYED) &&
            !player_isFlag(PLAYER_CREATING)) { /* update position */

         /* Calculate some dimensions. */
         w  = (SCREEN_W + 2.*STAR_BUF);
         w += conf.zoom_stars * (w / conf.zoom_far - 1.);
         h  = (SCREEN_H + 2.*STAR_BUF);
         h += conf.zoom_stars * (h / conf.zoom_far - 1.);
         hw = w/2.;
         hh = h/2.;

         /* Calculate new star positions. */
         for (i=0; i < nstars; i++) {

            /* calculate new position */
            b = 9. - 10.*star_colour[8*i+3];
            star_vertex[4*i+0] = star_vertex[4*i+0] -
               (GLfloat)player.p->solid->vel.x / b*(GLfloat)dt;
            star_vertex[4*i+1] = star_vertex[4*i+1] -
               (GLfloat)player.p->solid->vel.y / b*(GLfloat)dt;

            /* check boundries */
            if (star_vertex[4*i+0] > hw)
               star_vertex[4*i+0] -= w;
            else if (star_vertex[4*i+0] < -hw)
               star_vertex[4*i+0] += w;
            if (star_vertex[4*i+1] > hh)
               star_vertex[4*i+1] -= h;
            else if (star_vertex[4*i+1] < -hh)
               star_vertex[4*i+1] += h;
         }

         /* Upload the data. */
         gl_vboSubData( star_vertexVBO, 0, nstars * 4 * sizeof(GLfloat), star_vertex );
      }

   if ((player.p != NULL) && !player_isFlag(PLAYER_DESTROYED) &&
         !player_isFlag(PLAYER_CREATING) &&
         pilot_isFlag(player.p,PILOT_HYPERSPACE) && /* hyperspace fancy effects */
         (player.p->ptimer < HYPERSPACE_STARS_BLUR)) {

      glShadeModel(GL_SMOOTH);

      /* lines will be based on velocity */
      m  = HYPERSPACE_STARS_BLUR-player.p->ptimer;
      m /= HYPERSPACE_STARS_BLUR;
      m *= HYPERSPACE_STARS_LENGTH;
      x = m*cos(VANGLE(player.p->solid->vel));
      y = m*sin(VANGLE(player.p->solid->vel));

      /* Generate lines. */
      for (i=0; i < nstars; i++) {
         brightness = star_colour[8*i+3];
         star_vertex[4*i+2] = star_vertex[4*i+0] + x*brightness;
         star_vertex[4*i+3] = star_vertex[4*i+1] + y*brightness;
      }

      /* Draw the lines. */
      gl_vboSubData( star_vertexVBO, 0, nstars * 4 * sizeof(GLfloat), star_vertex );
      gl_vboActivate( star_vertexVBO, GL_VERTEX_ARRAY, 2, GL_FLOAT, 0 );
      gl_vboActivate( star_colourVBO, GL_COLOR_ARRAY,  4, GL_FLOAT, 0 );
      glDrawArrays( GL_LINES, 0, nstars );

      glShadeModel(GL_FLAT);
   }
   else { /* normal rendering */
      /* Render. */
      gl_vboActivate( star_vertexVBO, GL_VERTEX_ARRAY, 2, GL_FLOAT, 2 * sizeof(GLfloat) );
      gl_vboActivate( star_colourVBO, GL_COLOR_ARRAY,  4, GL_FLOAT, 4 * sizeof(GLfloat) );
      glDrawArrays( GL_POINTS, 0, nstars );
      gl_checkErr();
   }

   /* Disable vertex array. */
   gl_vboDeactivate();

   /* Pop matrix. */
   gl_matrixPop();
}


/**
 * @brief Render the background.
 */
void background_render( double dt )
{
   background_renderImages();
   background_renderStars(dt);
}


/**
 * @brief Adds a new background image.
 */
int background_addImage( glTexture *image, double x, double y,
      double move, double scale, glColour *col )
{
   background_image_t *bkg;

   /* See if must create. */
   if (bkg_image_arr == NULL)
      bkg_image_arr = array_create( background_image_t );

   /* Create image. */
   bkg         = &array_grow( &bkg_image_arr );
   bkg->image  = gl_dupTexture(image);
   bkg->x      = x;
   bkg->y      = y;
   bkg->move   = move;
   bkg->scale  = scale;
   memcpy( &bkg->col, (col!=NULL) ? col : &cWhite, sizeof(glColour) );


   return array_size(bkg_image_arr)-1;
}


/**
 * @brief Renders the background images.
 */
static void background_renderImages (void)
{
   int i;
   background_image_t *bkg;
   double px,py, x,y, xs,ys, z;

   /* Must have an image array created. */
   if (bkg_image_arr == NULL)
      return;

   /* Render images in order. */
   for (i=0; i<array_size(bkg_image_arr); i++) {
      bkg = &bkg_image_arr[i];

      gl_cameraGet( &px, &py );
      x  = px + (bkg->x - px) * bkg->move - bkg->scale*bkg->image->sw/2.;
      y  = py + (bkg->y - py) * bkg->move - bkg->scale*bkg->image->sh/2.;
      gl_gameToScreenCoords( &xs, &ys, x, y );
      gl_cameraZoomGet( &z );
      z *= bkg->scale;
      gl_blitScale( bkg->image, xs, ys,
            z*bkg->image->sw, z*bkg->image->sh, &bkg->col );
   }
}


/**
 * @brief Creates a background Lua state from a script.
 */
static lua_State* background_create( const char *name )
{
   uint32_t bufsize;
   char path[PATH_MAX];
   char *buf;
   lua_State *L;

   /* Create file name. */
   snprintf( path, sizeof(path), "dat/bkg/%s.lua", name );

   /* Create the Lua state. */
   L = nlua_newState();
   nlua_loadStandard(L,1);
   nlua_loadTex(L,0);
   nlua_loadCol(L,0);
   nlua_loadBackground(L,0);

   /* Open file. */
   buf = ndata_read( path, &bufsize );
   if (buf == NULL) {
      WARN("Default background script '%s' not found.", path);
      lua_close(L);
      return NULL;
   }

   /* Load file. */
   if (luaL_dobuffer(L, buf, bufsize, path) != 0) {
      WARN("Error loading background file: %s\n"
            "%s\n"
            "Most likely Lua file has improper syntax, please check",
            path, lua_tostring(L,-1));
      free(buf);
      lua_close(L);
      return NULL;
   }
   free(buf);

   return L;
}


/**
 * @brief Initializes the background system.
 */
int background_init (void)
{
   /* Load Lua. */
   bkg_def_L = background_create( "default" );
   return 0;
}


/**
 * @brief Loads a background script by name.
 */
int background_load( const char *name )
{
   int ret;
   lua_State *L;
   const char *err;

   /* Free if exists. */
   background_clearCurrent();

   /* Load default. */
   if (name == NULL)
      bkg_cur_L = bkg_def_L;
   /* Load new script. */
   else
      bkg_cur_L = background_create( name );

   /* Comfort. */
   L = bkg_cur_L;
   if (L == NULL)
      return -1;

   /* Run Lua. */
   ret = 0;
   lua_getglobal(L,"background");
   ret = lua_pcall(L, 0, 0, 0);
   if (ret != 0) { /* error has occured */
      err = (lua_isstring(L,-1)) ? lua_tostring(L,-1) : NULL;
      WARN("Background -> 'background' : %s",
            (err) ? err : "unknown error");
      lua_pop(L, 1);
      ret = -1;
   }
   return 0;
}


/**
 * @brief Destroys the current running background script.
 */
static void background_clearCurrent (void)
{
   if (bkg_cur_L != bkg_def_L) {
      if (bkg_cur_L != NULL)
         lua_close( bkg_cur_L );
   }
   bkg_cur_L = NULL;
}


/**
 * @brief Cleans up the background stuff.
 */
void background_clear (void)
{
   int i;
   background_image_t *bkg;

   /* Destroy current background script. */
   background_clearCurrent();

   /* Must have an image array created. */
   if (bkg_image_arr == NULL)
      return;

   for (i=0; i<array_size(bkg_image_arr); i++) {
      bkg = &bkg_image_arr[i];
      gl_freeTexture( bkg->image );
   }

   /* Erase it all. */
   array_erase( &bkg_image_arr, &bkg_image_arr[0], &bkg_image_arr[ array_size(bkg_image_arr) ] );
}


/**
 * @brief Cleans up and frees memory after the backgrounds.
 */
void background_free (void)
{
   /* Free the Lua. */
   background_clear();
   if (bkg_def_L != NULL)
      lua_close( bkg_def_L );
   bkg_def_L = NULL;

   /* Free the images. */
   if (bkg_image_arr != NULL) {
      array_free( bkg_image_arr );
      bkg_image_arr = NULL;
   }

   /* Free the Lua. */
   if (bkg_cur_L == bkg_def_L) {
      if (bkg_cur_L != NULL)
         lua_close( bkg_cur_L );
      bkg_cur_L = NULL;
      bkg_def_L = NULL;
   }
   else {
      if (bkg_cur_L != NULL)
         lua_close( bkg_cur_L );
      if (bkg_def_L != NULL)
         lua_close( bkg_def_L );
      bkg_cur_L = NULL;
      bkg_def_L = NULL;
   }

   /* Free the stars. */
   if (star_vertex) {
      free(star_vertex);
      star_vertex = NULL;
   }
   if (star_colour) {
      free(star_colour);
      star_colour = NULL;
   }
   nstars = 0;
   mstars = 0;
}

