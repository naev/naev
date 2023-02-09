/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file background.c
 *
 * @brief Handles displaying backgrounds.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "background.h"

#include "array.h"
#include "camera.h"
#include "conf.h"
#include "log.h"
#include "gui.h"
#include "ndata.h"
#include "nebula.h"
#include "nlua.h"
#include "nlua_audio.h"
#include "nlua_bkg.h"
#include "nlua_colour.h"
#include "nlua_tex.h"
#include "nlua_camera.h"
#include "nlua_gfx.h"
#include "nluadef.h"
#include "nstring.h"
#include "nxml.h"
#include "opengl.h"
#include "pause.h"
#include "player.h"
#include "rng.h"

/**
 * @brief Represents a background image like say a Nebula.
 */
typedef struct background_image_s {
   unsigned int id; /**< Background id. */
   glTexture *image; /**< Image to display. */
   double x; /**< X center of the image. */
   double y; /**< Y center of the image. */
   double move; /**< How many pixels it moves for each pixel the player moves. */
   double scale; /**< How the image should be scaled. */
   double angle; /**< Rotation (in radians). */
   glColour col; /**< Colour to use. */
} background_image_t;
static background_image_t *bkg_image_arr_bk = NULL; /**< Background image array to display (behind dust). */
static background_image_t *bkg_image_arr_ft = NULL; /**< Background image array to display (in front of dust). */

static unsigned int bkg_idgen = 0; /**< ID generator for backgrounds. */

/**
 * @brief Backgrounds.
 */
static nlua_env bkg_cur_env = LUA_NOREF; /**< Current Lua state. */
static nlua_env bkg_def_env = LUA_NOREF; /**< Default Lua state. */
static int bkg_L_renderbg = LUA_NOREF; /**< Background rendering function. */
static int bkg_L_rendermg = LUA_NOREF; /**< Middleground rendering function. */
static int bkg_L_renderfg = LUA_NOREF; /**< Foreground rendering function. */
static int bkg_L_renderov = LUA_NOREF; /**< Overlay rendering function. */

/*
 * Background dust.
 */
#define STAR_BUF     250 /**< Area to leave around screen for dust, more = less repetition */
static gl_vbo *dust_vertexVBO = NULL; /**< Star Vertex VBO. */
static unsigned int ndust = 0; /**< Total dust. */
static GLfloat dust_x = 0.; /**< Star X movement. */
static GLfloat dust_y = 0.; /**< Star Y movement. */

/*
 * Prototypes.
 */
static void background_renderImages( background_image_t *bkg_arr );
static nlua_env background_create( const char *path );
static void background_clearCurrent (void);
static void background_clearImgArr( background_image_t **arr );
/* Sorting. */
static int bkg_compare( const void *p1, const void *p2 );
static void bkg_sort( background_image_t *arr );

/**
 * @brief Initializes background dust.
 *
 *    @param n Number of dust to add (dust per 800x640 screen).
 */
void background_initDust( int n )
{
   GLfloat w, h, hw, hh;
   double size;
   GLfloat *dust_vertex;

   /* Calculate size. */
   size  = SCREEN_W*SCREEN_H+STAR_BUF*STAR_BUF;
   size /= pow2(conf.zoom_far);

   /* Calculate dust buffer. */
   w  = (SCREEN_W + 2.*STAR_BUF);
   w += (w / conf.zoom_far - 1.);
   h  = (SCREEN_H + 2.*STAR_BUF);
   h += (h / conf.zoom_far - 1.);
   hw = w / 2.;
   hh = h / 2.;

   /* Calculate dust. */
   size  *= n;
   ndust = (unsigned int)(size/(800.*600.));

   /* Create data. */
   dust_vertex = malloc( ndust * sizeof(GLfloat) * 6 );

   for (unsigned int i=0; i < ndust; i++) {
      /* Set the position. */
      dust_vertex[6*i+0] = RNGF()*w - hw;
      dust_vertex[6*i+1] = RNGF()*h - hh;
      dust_vertex[6*i+3] = dust_vertex[6*i+0];
      dust_vertex[6*i+4] = dust_vertex[6*i+1];
      /* Set the colour. */
      dust_vertex[6*i+2] = RNGF()*0.6 + 0.2;
      dust_vertex[6*i+5] = dust_vertex[6*i+2];
   }

   /* Recreate VBO. */
   gl_vboDestroy( dust_vertexVBO );
   dust_vertexVBO = gl_vboCreateStatic(
         ndust * sizeof(GLfloat) * 6, dust_vertex );

   free(dust_vertex);
}

/**
 * @brief Displaces the dust, useful with camera.
 */
void background_moveDust( double x, double y )
{
   dust_x += (GLfloat) x;
   dust_y += (GLfloat) y;
}

/**
 * @brief Renders the dustry background.
 *
 *    @param dt Current delta tick.
 */
void background_renderDust( const double dt )
{
   (void) dt;
   GLfloat x, y, h, w, m;
   double z;
   mat4 projection;
   int points = 1;

   /* Do some scaling for now. */
   z = cam_getZoom();
   m = 1.;
   projection = gl_view_matrix;
   mat4_translate( &projection, SCREEN_W/2., SCREEN_H/2., 0 );
   mat4_scale( &projection, z, z, 1 );

   /* Decide on shade mode. */
   if ((player.p != NULL) && !player_isFlag(PLAYER_DESTROYED) &&
         !player_isFlag(PLAYER_CREATING)) {
      double dx, dy, vmod;

      /* Get camera movement. */
      cam_getVel( &dx, &dy );
      vmod = hypot( dx, dy );

      if (pilot_isFlag(player.p,PILOT_HYPERSPACE)) { /* hyperspace fancy effects */
         /* lines get longer the closer we are to finishing the jump */
         m = MAX( 0, HYPERSPACE_DUST_BLUR-player.p->ptimer );
         m /= HYPERSPACE_DUST_BLUR;
         m *= HYPERSPACE_DUST_LENGTH;
         if (m > 1.) {
            double angle = atan2( dy, dx );
            x = m * cos( angle );
            y = m * sin( angle );
            points = 0;
         }
      }
      else if (dt_mod * vmod > 500. ) {
         /* Very short lines tend to flicker horribly. A stock Llama at 2x
          * speed just so happens to make very short lines. A 5px minimum
          * is long enough to (mostly) alleviate the flickering. */
         /* TODO don't use GL_LINES. */
         double angle = atan2( dy, dx );
         m = MAX( 5., dt_mod * vmod/25. - 20 );
         x = m * cos( angle );
         y = m * sin( angle );
         points = 0;
      }
   }

   /* Calculate some dimensions. */
   w  = (SCREEN_W + 2.*STAR_BUF);
   w += (w / conf.zoom_far - 1.);
   h  = (SCREEN_H + 2.*STAR_BUF);
   h += (h / conf.zoom_far - 1.);

   /* Common shader stuff. */
   glUseProgram(shaders.dust.program);
   gl_uniformMat4(shaders.dust.projection, &projection);
   glUniform2f(shaders.dust.offset_xy, dust_x, dust_y);
   glUniform3f(shaders.dust.dims, w, h, 1. / gl_screen.scale);
   glUniform1i(shaders.dust.use_lines, !points);
   glUniform1f(shaders.dust.dim, CLAMP(0.5, 1., 1.-(m-1.)/25.)*z );

   /* Vertices. */
   glEnableVertexAttribArray( shaders.dust.vertex );
   glEnableVertexAttribArray( shaders.dust.brightness );

   /* Set up the vertices. */
   if (points) {
      gl_vboActivateAttribOffset( dust_vertexVBO, shaders.dust.vertex, 0,
            2, GL_FLOAT, 6 * sizeof(GLfloat) );
      gl_vboActivateAttribOffset( dust_vertexVBO, shaders.dust.brightness, 2 * sizeof(GLfloat),
            1, GL_FLOAT, 6 * sizeof(GLfloat) );
      glUniform2f(shaders.dust.xy, 0., 0.);
      glDrawArrays( GL_POINTS, 0, ndust/2 );
   }
   else {
      gl_vboActivateAttribOffset( dust_vertexVBO, shaders.dust.vertex, 0,
            2, GL_FLOAT, 3 * sizeof(GLfloat) );
      gl_vboActivateAttribOffset( dust_vertexVBO, shaders.dust.brightness, 2 * sizeof(GLfloat),
            1, GL_FLOAT, 3 * sizeof(GLfloat) );
      glUniform2f(shaders.dust.xy, x, y);
      glDrawArrays( GL_LINES, 0, ndust );
   }

   /* Disable vertex array. */
   glDisableVertexAttribArray( shaders.dust.vertex );
   glDisableVertexAttribArray( shaders.dust.brightness );

   glUseProgram(0);

   /* Check for errors. */
   gl_checkErr();
}

/**
 * @brief Render the background.
 *
 *    @param dt Real delta ticks elapsed.
 */
void background_render( double dt )
{
   if (bkg_L_renderbg != LUA_NOREF) {
      lua_rawgeti( naevL, LUA_REGISTRYINDEX, bkg_L_renderbg );
      lua_pushnumber( naevL, dt ); /* Note that this is real_dt. */
      if (nlua_pcall( bkg_cur_env, 1, 0 )) {
         WARN( _("Background script 'renderbg' error:\n%s"), lua_tostring(naevL,-1));
         lua_pop( naevL, 1 );
      }
   }

   background_renderImages( bkg_image_arr_bk );

   if (bkg_L_rendermg != LUA_NOREF) {
      lua_rawgeti( naevL, LUA_REGISTRYINDEX, bkg_L_rendermg );
      lua_pushnumber( naevL, dt ); /* Note that this is real_dt. */
      if (nlua_pcall( bkg_cur_env, 1, 0 )) {
         WARN( _("Background script 'rendermg' error:\n%s"), lua_tostring(naevL,-1));
         lua_pop( naevL, 1 );
      }
   }

   background_renderDust(dt);
   background_renderImages( bkg_image_arr_ft );

   if (bkg_L_renderfg != LUA_NOREF) {
      lua_rawgeti( naevL, LUA_REGISTRYINDEX, bkg_L_renderfg );
      lua_pushnumber( naevL, dt ); /* Note that this is real_dt. */
      if (nlua_pcall( bkg_cur_env, 1, 0 )) {
         WARN( _("Background script 'renderfg' error:\n%s"), lua_tostring(naevL,-1));
         lua_pop( naevL, 1 );
      }
   }
}

/**
 * @brief Renders the background overlay.
 */
void background_renderOverlay( double dt )
{
   if (bkg_L_renderov != LUA_NOREF) {
      lua_rawgeti( naevL, LUA_REGISTRYINDEX, bkg_L_renderov );
      lua_pushnumber( naevL, dt ); /* Note that this is real_dt. */
      if (nlua_pcall( bkg_cur_env, 1, 0 )) {
         WARN( _("Background script 'renderov' error:\n%s"), lua_tostring(naevL,-1));
         lua_pop( naevL, 1 );
      }
   }
}

/**
 * @brief Compares two different backgrounds and sorts them.
 */
static int bkg_compare( const void *p1, const void *p2 )
{
   background_image_t *bkg1, *bkg2;

   bkg1 = (background_image_t*) p1;
   bkg2 = (background_image_t*) p2;

   if (bkg1->move < bkg2->move)
      return -1;
   else if (bkg1->move > bkg2->move)
      return +1;
   return  0;
}

/**
 * @brief Sorts the backgrounds by movement.
 */
static void bkg_sort( background_image_t *arr )
{
   qsort( arr, array_size(arr), sizeof(background_image_t), bkg_compare );
}

/**
 * @brief Adds a new background image.
 */
unsigned int background_addImage( glTexture *image, double x, double y,
      double move, double scale, double angle, const glColour *col, int foreground )
{
   background_image_t *bkg, **arr;

   if (foreground)
      arr = &bkg_image_arr_ft;
   else
      arr = &bkg_image_arr_bk;

   /* See if must create. */
   if (*arr == NULL)
      *arr = array_create( background_image_t );

   /* Create image. */
   bkg         = &array_grow( arr );
   bkg->id     = ++bkg_idgen;
   bkg->image  = gl_dupTexture(image);
   bkg->x      = x;
   bkg->y      = y;
   bkg->move   = move;
   bkg->scale  = scale;
   bkg->angle  = angle;
   bkg->col    = (col!=NULL) ? *col : cWhite;

   /* Sort if necessary. */
   bkg_sort( *arr );

   return bkg_idgen;
}

/**
 * @brief Renders the background images.
 */
static void background_renderImages( background_image_t *bkg_arr )
{
   /* Skip rendering altogether if disabled. */
   if (conf.bg_brightness <= 0.)
      return;

   /* Render images in order. */
   for (int i=0; i<array_size(bkg_arr); i++) {
      double cx,cy, x,y, gx,gy, z, m;
      glColour col;
      background_image_t *bkg = &bkg_arr[i];

      cam_getPos( &cx, &cy );
      gui_getOffset( &gx, &gy );
      m = bkg->move;
      z = bkg->scale;
      x  = (bkg->x - cx) * m - z*bkg->image->sw/2. + gx + SCREEN_W/2.;
      y  = (bkg->y - cy) * m - z*bkg->image->sh/2. + gy + SCREEN_H/2.;

      col.r = bkg->col.r * conf.bg_brightness;
      col.g = bkg->col.g * conf.bg_brightness;
      col.b = bkg->col.b * conf.bg_brightness;
      col.a = bkg->col.a;

      gl_renderTexture( bkg->image, x, y, z*bkg->image->sw, z*bkg->image->sh, 0., 0., bkg->image->srw, bkg->image->srh, &col, bkg->angle );
   }
}

/**
 * @brief Creates a background Lua state from a script.
 */
static nlua_env background_create( const char *name )
{
   size_t bufsize;
   char path[PATH_MAX];
   char *buf;
   nlua_env env;

   /* Create file name. */
   snprintf( path, sizeof(path), BACKGROUND_PATH"%s.lua", name );

   /* Create the Lua env. */
   env = nlua_newEnv();
   nlua_loadStandard(env);
   nlua_loadTex(env);
   nlua_loadCol(env);
   nlua_loadBackground(env);
   nlua_loadCamera(env);
   nlua_loadGFX(env);

   /* Open file. */
   buf = ndata_read( path, &bufsize );
   if (buf == NULL) {
      WARN( _("Background script '%s' not found."), path);
      nlua_freeEnv(env);
      return LUA_NOREF;
   }

   /* Load file. */
   if (nlua_dobufenv(env, buf, bufsize, path) != 0) {
      WARN( _("Error loading background file: %s\n"
            "%s\n"
            "Most likely Lua file has improper syntax, please check"),
            path, lua_tostring(naevL,-1));
      free(buf);
      nlua_freeEnv(env);
      return LUA_NOREF;
   }
   free(buf);

   return env;
}

/**
 * @brief Initializes the background system.
 */
int background_init (void)
{
   /* Load Lua. */
   bkg_def_env = background_create( "default" );
   return 0;
}

/**
 * @brief Loads a background script by name.
 */
int background_load( const char *name )
{
   int ret;
   nlua_env env;

   /* Free if exists. */
   background_clear();

   /* Load default. */
   if (name == NULL)
      bkg_cur_env = bkg_def_env;
   /* Load new script. */
   else
      bkg_cur_env = background_create( name );

   /* Comfort. */
   env = bkg_cur_env;
   if (env == LUA_NOREF)
      return -1;

   /* Run Lua. */
   nlua_getenv(naevL, env,"background");
   ret = nlua_pcall(env, 0, 0);
   if (ret != 0) { /* error has occurred */
      const char *err = (lua_isstring(naevL,-1)) ? lua_tostring(naevL,-1) : NULL;
      WARN( _("Background -> 'background' : %s"),
            (err) ? err : _("unknown error"));
      lua_pop(naevL, 1);
   }

   /* See if there are render functions. */
   bkg_L_renderbg = nlua_refenv( env, "renderbg" );
   bkg_L_rendermg = nlua_refenv( env, "rendermg" );
   bkg_L_renderfg = nlua_refenv( env, "renderfg" );
   bkg_L_renderov = nlua_refenv( env, "renderov" );

   return ret;
}

/**
 * @brief Destroys the current running background script.
 */
static void background_clearCurrent (void)
{
   if (bkg_cur_env != bkg_def_env) {
      nlua_freeEnv( bkg_cur_env );
   }
   bkg_cur_env = LUA_NOREF;

   luaL_unref( naevL, LUA_REGISTRYINDEX, bkg_L_renderbg );
   luaL_unref( naevL, LUA_REGISTRYINDEX, bkg_L_rendermg );
   luaL_unref( naevL, LUA_REGISTRYINDEX, bkg_L_renderfg );
   luaL_unref( naevL, LUA_REGISTRYINDEX, bkg_L_renderov );
   bkg_L_renderbg = LUA_NOREF;
   bkg_L_rendermg = LUA_NOREF;
   bkg_L_renderfg = LUA_NOREF;
   bkg_L_renderov = LUA_NOREF;
}

/**
 * @brief Cleans up the background stuff.
 */
void background_clear (void)
{
   /* Destroy current background script. */
   background_clearCurrent();

   /* Clear the backgrounds. */
   background_clearImgArr( &bkg_image_arr_bk );
   background_clearImgArr( &bkg_image_arr_ft );
}

/**
 * @brief Clears a background image array.
 *
 *    @param arr Array to clear.
 */
static void background_clearImgArr( background_image_t **arr )
{
   for (int i=0; i<array_size(*arr); i++) {
      background_image_t *bkg = &((*arr)[i]);
      gl_freeTexture( bkg->image );
   }

   /* Erase it all. */
   array_erase( arr, array_begin(*arr), array_end(*arr) );
}

/**
 * @brief Cleans up and frees memory after the backgrounds.
 */
void background_free (void)
{
   /* Free the Lua. */
   background_clear();
   nlua_freeEnv( bkg_def_env );
   bkg_def_env = LUA_NOREF;

   /* Free the images. */
   array_free( bkg_image_arr_ft );
   bkg_image_arr_ft = NULL;
   array_free( bkg_image_arr_bk );
   bkg_image_arr_bk = NULL;

   /* Free the Lua. */
   nlua_freeEnv( bkg_cur_env );
   bkg_cur_env = LUA_NOREF;

   gl_vboDestroy( dust_vertexVBO );
   dust_vertexVBO = NULL;

   ndust = 0;
}

/**
 * @brief returns the background images, and number of these
 */
glTexture** background_getTextures (void)
{
  glTexture **imgs = array_create_size( glTexture*, array_size( bkg_image_arr_bk ));
  for (int i=0; i<array_size(bkg_image_arr_bk); i++)
    array_push_back( &imgs, gl_dupTexture(bkg_image_arr_bk[i].image) );
  return imgs;
}
