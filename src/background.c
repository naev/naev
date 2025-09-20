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
#include "ndata.h"
#include "nlua.h"
#include "nlua_bkg.h"
#include "nlua_camera.h"
#include "nlua_colour.h"
#include "nlua_gfx.h"
#include "nlua_tex.h"
#include "ntracing.h"
#include "opengl.h"
#include "pause.h"
#include "player.h"
#include "rng.h"

/**
 * @brief Represents a background image like say a Nebula.
 */
typedef struct background_image_s {
   unsigned int id;    /**< Background id. */
   glTexture   *image; /**< Image to display. */
   double       x;     /**< X centre of the image. */
   double       y;     /**< Y centre of the image. */
   double
      move; /**< How many pixels it moves for each pixel the player moves. */
   double   scale;     /**< How the image should be scaled. */
   double   angle;     /**< Rotation (in radians). */
   glColour col;       /**< Colour to use. */
   glColour radiosity; /**< Radiosity. */

   /* Handled during rendering. */
   int   L_idx; /**< Lighting index. Invalid if < 0. */
   Light L;     /**< Lighting to use. */
} background_image_t;
static background_image_t *bkg_image_arr_bk =
   NULL; /**< Background image array to display (behind dust). Assumed to be a
            debris layer. */
static background_image_t *bkg_image_arr_ft =
   NULL; /**< Background image array to display (in front of dust). Assumed to
            be a star layer. */

static unsigned int bkg_idgen = 0; /**< ID generator for backgrounds. */

/**
 * @brief Backgrounds.
 */
static nlua_env *bkg_cur_env = NULL;      /**< Current Lua state. */
static nlua_env *bkg_def_env = NULL;      /**< Default Lua state. */
static int bkg_L_renderbg    = LUA_NOREF; /**< Background rendering function. */
static int bkg_L_rendermg = LUA_NOREF; /**< Middleground rendering function. */
static int bkg_L_renderfg = LUA_NOREF; /**< Foreground rendering function. */
static int bkg_L_renderov = LUA_NOREF; /**< Overlay rendering function. */

/*
 * Background dust.
 */
#define STAR_BUF                                                               \
   250 /**< Area to leave around screen for dust, more = less repetition */
static gl_vbo      *dust_vertexVBO = NULL; /**< Star Vertex VBO. */
static unsigned int ndust          = 0;    /**< Total dust. */
static GLfloat      dust_x         = 0.;   /**< Star X movement. */
static GLfloat      dust_y         = 0.;   /**< Star Y movement. */

/*
 * Prototypes.
 */
static void      background_renderImages( background_image_t *bkg_arr );
static nlua_env *background_create( const char *path );
static void      background_clearCurrent( void );
static void      background_clearImgArr( background_image_t **arr );
/* Sorting. */
static int  bkg_compare( const void *p1, const void *p2 );
static void bkg_sort( background_image_t *arr );

/**
 * @brief Initializes background dust.
 *
 *    @param n Number of dust to add (dust per 800x640 screen).
 */
void background_initDust( int n )
{
   GLfloat  w, h, hw, hh;
   double   size;
   GLfloat *dust_vertex;

   NTracingZone( _ctx, 1 );

   /* Calculate size. */
   size = SCREEN_W * SCREEN_H + STAR_BUF * STAR_BUF;
   size /= pow2( conf.zoom_far );

   /* Calculate dust buffer. */
   w = ( SCREEN_W + 2. * STAR_BUF );
   w += ( w / conf.zoom_far - 1. );
   h = ( SCREEN_H + 2. * STAR_BUF );
   h += ( h / conf.zoom_far - 1. );
   hw = w / 2.;
   hh = h / 2.;

   /* Calculate dust. */
   size *= n;
   ndust = (unsigned int)( size / ( 800. * 600. ) );

   /* Create data. */
   dust_vertex = malloc( ndust * sizeof( GLfloat ) * 3 );

   for ( unsigned int i = 0; i < ndust; i++ ) {
      /* Set the position. */
      dust_vertex[3 * i + 0] = RNGF() * w - hw;
      dust_vertex[3 * i + 1] = RNGF() * h - hh;
      /* Set the colour. */
      dust_vertex[3 * i + 2] = RNGF() * 0.6 + 0.2;
   }

   /* Recreate VBO. */
   gl_vboDestroy( dust_vertexVBO );
   dust_vertexVBO =
      gl_vboCreateStatic( ndust * sizeof( GLfloat ) * 3, dust_vertex );

   free( dust_vertex );

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Displaces the dust, useful with camera.
 */
void background_moveDust( double x, double y )
{
   dust_x += (GLfloat)x;
   dust_y += (GLfloat)y;
}

/**
 * @brief Renders the dustry background.
 *
 *    @param dt Current delta tick.
 */
void background_renderDust( const double dt )
{
   (void)dt;
   GLfloat h, w, m;
   double  z, angle;
   mat4    projection;
   int     points = 1;

   NTracingZone( _ctx, 1 );

   /* Do some scaling for now. */
   z          = cam_getZoom();
   m          = 1.;
   angle      = 0.;
   projection = gl_view_matrix;
   mat4_translate_scale_xy( &projection, SCREEN_W / 2., SCREEN_H / 2., z, z );

   /* Decide on shade mode. */
   if ( ( player.p != NULL ) && !player_isFlag( PLAYER_DESTROYED ) &&
        !player_isFlag( PLAYER_CREATING ) ) {
      double dx, dy, vmod;

      /* Get camera movement. */
      cam_getVel( &dx, &dy );
      vmod = hypot( dx, dy );

      if ( pilot_isFlag( player.p,
                         PILOT_HYPERSPACE ) ) { /* hyperspace fancy effects */
         /* lines get longer the closer we are to finishing the jump */
         m = MAX( 0, HYPERSPACE_DUST_BLUR - player.p->ptimer );
         if ( m > 0. ) {
            m *= HYPERSPACE_DUST_LENGTH / HYPERSPACE_DUST_BLUR;
            angle  = atan2( dy, dx );
            points = 0;
         }
      } else if ( dt_mod * vmod > 500. ) {
         angle  = atan2( dy, dx );
         m      = ( dt_mod * vmod ) / 25. - 20.;
         points = 0;
      }
   }

   /* Calculate some dimensions. */
   w = ( SCREEN_W + 2. * STAR_BUF );
   w += ( w / conf.zoom_far - 1. );
   h = ( SCREEN_H + 2. * STAR_BUF );
   h += ( h / conf.zoom_far - 1. );

   /* Common shader stuff. */
   glUseProgram( shaders.dust.program );
   gl_uniformMat4( shaders.dust.projection, &projection );
   glUniform2f( shaders.dust.offset_xy, dust_x, dust_y );
   z = 0.5 * z / gl_screen.scale;
   if ( points ) {
      glUniform3f( shaders.dust.dims, MAX( 0.5, m * z ), 0., 0. );
   } else {
      double p = MAX( 0.5, z ) * MAX( 0.5, ( 1. - m / 40. ) );
      glUniform3f( shaders.dust.dims, p, angle, m );
   }
   glUniform3f( shaders.dust.screen, w, h, 1. / gl_screen.scale );
   glUniform1i( shaders.dust.use_lines, !points );

   /* Vertices. */
   glEnableVertexAttribArray( shaders.dust.shape );
   glEnableVertexAttribArray( shaders.dust.vertex );
   glEnableVertexAttribArray( shaders.dust.brightness );

   /* Set up the vertices. */
   gl_vboActivateAttribOffset( gl_circleVBO, shaders.dust.shape, 0, 2, GL_FLOAT,
                               0 );
   gl_vboActivateAttribOffset( dust_vertexVBO, shaders.dust.vertex, 0, 2,
                               GL_FLOAT, 3 * sizeof( GLfloat ) );
   gl_vboActivateAttribOffset( dust_vertexVBO, shaders.dust.brightness,
                               2 * sizeof( GLfloat ), 1, GL_FLOAT,
                               3 * sizeof( GLfloat ) );

   glVertexAttribDivisor( shaders.dust.shape, 0 );
   glVertexAttribDivisor( shaders.dust.vertex, 1 );
   glVertexAttribDivisor( shaders.dust.brightness, 1 );

   // glDrawArrays( GL_POINTS, 0, ndust );
   glDrawArraysInstanced( GL_TRIANGLE_STRIP, 0, 4, ndust );

   glVertexAttribDivisor( shaders.dust.shape, 0 );
   glVertexAttribDivisor( shaders.dust.vertex, 0 );
   glVertexAttribDivisor( shaders.dust.brightness, 0 );

   /* Disable vertex array. */
   glDisableVertexAttribArray( shaders.dust.shape );
   glDisableVertexAttribArray( shaders.dust.vertex );
   glDisableVertexAttribArray( shaders.dust.brightness );

   glUseProgram( 0 );

   /* Check for errors. */
   gl_checkErr();

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Render the background.
 *
 *    @param dt Real delta ticks elapsed.
 */
void background_render( double dt )
{
   NTracingZone( _ctx, 1 );

   if ( bkg_L_renderbg != LUA_NOREF ) {
      lua_rawgeti( naevL, LUA_REGISTRYINDEX, bkg_L_renderbg );
      lua_pushnumber( naevL, dt ); /* Note that this is real_dt. */
      if ( nlua_pcall( bkg_cur_env, 1, 0 ) ) {
         WARN( _( "Background script 'renderbg' error:\n%s" ),
               lua_tostring( naevL, -1 ) );
         lua_pop( naevL, 1 );
      }
   }

   background_renderImages( bkg_image_arr_bk );

   if ( bkg_L_rendermg != LUA_NOREF ) {
      lua_rawgeti( naevL, LUA_REGISTRYINDEX, bkg_L_rendermg );
      lua_pushnumber( naevL, dt ); /* Note that this is real_dt. */
      if ( nlua_pcall( bkg_cur_env, 1, 0 ) ) {
         WARN( _( "Background script 'rendermg' error:\n%s" ),
               lua_tostring( naevL, -1 ) );
         lua_pop( naevL, 1 );
      }
   }

   background_renderDust( dt );
   background_renderImages( bkg_image_arr_ft );

   if ( bkg_L_renderfg != LUA_NOREF ) {
      lua_rawgeti( naevL, LUA_REGISTRYINDEX, bkg_L_renderfg );
      lua_pushnumber( naevL, dt ); /* Note that this is real_dt. */
      if ( nlua_pcall( bkg_cur_env, 1, 0 ) ) {
         WARN( _( "Background script 'renderfg' error:\n%s" ),
               lua_tostring( naevL, -1 ) );
         lua_pop( naevL, 1 );
      }
   }

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Renders the background overlay.
 */
void background_renderOverlay( double dt )
{
   NTracingZone( _ctx, 1 );

   if ( bkg_L_renderov != LUA_NOREF ) {
      lua_rawgeti( naevL, LUA_REGISTRYINDEX, bkg_L_renderov );
      lua_pushnumber( naevL, dt ); /* Note that this is real_dt. */
      if ( nlua_pcall( bkg_cur_env, 1, 0 ) ) {
         WARN( _( "Background script 'renderov' error:\n%s" ),
               lua_tostring( naevL, -1 ) );
         lua_pop( naevL, 1 );
      }
   }

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Compares two different backgrounds and sorts them.
 */
static int bkg_compare( const void *p1, const void *p2 )
{
   const background_image_t *bkg1 = (background_image_t *)p1;
   const background_image_t *bkg2 = (background_image_t *)p2;
   return bkg1->move - bkg2->move;
}

/**
 * @brief Sorts the backgrounds by movement.
 */
static void bkg_sort( background_image_t *arr )
{
   qsort( arr, array_size( arr ), sizeof( background_image_t ), bkg_compare );
}

/**
 * @brief Adds a new background image.
 */
unsigned int background_addImage( const glTexture *image, double x, double y,
                                  double move, double scale, double angle,
                                  const glColour *col, int foreground,
                                  const glColour *radiosity )
{
   double              a, d;
   background_image_t *bkg, **arr;

   if ( foreground )
      arr = &bkg_image_arr_ft;
   else
      arr = &bkg_image_arr_bk;

   /* See if must create. */
   if ( *arr == NULL )
      *arr = array_create( background_image_t );

   /* Create image. */
   bkg = &array_grow( arr );
   memset( bkg, 0, sizeof( background_image_t ) );
   bkg->id        = ++bkg_idgen;
   bkg->image     = gl_dupTexture( image );
   bkg->x         = x;
   bkg->y         = y;
   bkg->move      = move;
   bkg->scale     = scale;
   bkg->angle     = angle;
   bkg->col       = ( col != NULL ) ? *col : cWhite;
   bkg->radiosity = *radiosity;
   bkg->L_idx     = -1; /* Disable lighting. */

   /* Deal with lighting. */
   a = bkg->radiosity.a;
   d = pow2( a * bkg->radiosity.r ) + pow2( a * bkg->radiosity.g ) +
       pow2( a * bkg->radiosity.b );
   if ( d > 1e-3 ) {
      /* Get index. */
      bkg->L_idx = gltf_numLights() - L_default_const.nlights;

      /* Normalize so RGB is unitary. Compensate modifying alpha. */
      bkg->radiosity.r /= d;
      bkg->radiosity.g /= d;
      bkg->radiosity.b /= d;
      bkg->radiosity.a *= d;

      /* Set up the new light. */
      bkg->L.sun         = 1;
      bkg->L.colour.v[0] = bkg->radiosity.r;
      bkg->L.colour.v[1] = bkg->radiosity.g;
      bkg->L.colour.v[2] = bkg->radiosity.b;
      bkg->L.intensity   = bkg->radiosity.a;
      double cx, cy, rx, ry;
      cam_getPos( &cx, &cy );
      /* Relative coordinates. */
      rx              = ( bkg->x - cx ) * bkg->move;
      ry              = ( bkg->y - cy ) * bkg->move;
      bkg->L.pos.v[0] = rx;
      bkg->L.pos.v[1] = 2. * hypot( rx, ry ) - 300.;
      bkg->L.pos.v[2] = ry;
      if ( gltf_lightSet( bkg->L_idx, &bkg->L ) )
         bkg->L_idx = -1; /* Failed to add. */
   }

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
   if ( conf.bg_brightness <= 0. )
      return;

   /* Render images in order. */
   for ( int i = 0; i < array_size( bkg_arr ); i++ ) {
      double              cx, cy, x, y, rx, ry, z, m, s;
      glColour            col;
      background_image_t *bkg = &bkg_arr[i];

      cam_getPos( &cx, &cy );
      // z = 1. + bkg->move * cam_getZoom(); /* Perspective. */
      // z = cam_getZoom(); /* Orthorgonal. */
      // z = cam_getZoom() * (0.5 + 0.5 * bkg->move) + 0.5; /* Average
      // Orthogonal + Perspective. */
      z = cam_getZoom() * ( 0.2 + 0.8 * bkg->move ) +
          0.8; /* 20% Orthogonal, 80% Perspective. */
      m = z * bkg->move;
      s = z * bkg->scale;

      /* Relative coordinates. */
      rx = ( bkg->x - cx ) * m;
      ry = ( bkg->y - cy ) * m;
      /* Screen coordinates. */
      y = ry + SCREEN_H / 2. - z * tex_sw( bkg->image ) / 2.;
      x = rx + SCREEN_W / 2. - z * tex_sh( bkg->image ) / 2.;

      /* TODO speed up by not rendering when offscreen. */

      col.r = bkg->col.r * conf.bg_brightness;
      col.g = bkg->col.g * conf.bg_brightness;
      col.b = bkg->col.b * conf.bg_brightness;
      col.a = bkg->col.a;
      gl_renderTexture( bkg->image, x, y, s * tex_sw( bkg->image ),
                        s * tex_sh( bkg->image ), 0., 0., tex_srw( bkg->image ),
                        tex_srh( bkg->image ), &col, bkg->angle );

      /* See if we have to update scene lighting. */
      if ( bkg->L_idx >= 0 ) {
         double d = hypot( rx, ry );
         /* Update Light. */
         /* Not scaling light based on distance and assuming it is constant.
         double w = 1. / sqrt(3.);
         double a = CLAMP( 0., 1., d/5000. );
         bkg->L.colour.v[0] = bkg->radiosity.r * (1.-a) + a*w;
         bkg->L.colour.v[1] = bkg->radiosity.g * (1.-a) + a*w;
         bkg->L.colour.v[2] = bkg->radiosity.b * (1.-a) + a*w;
         */
         bkg->L.pos.v[0] = rx;
         bkg->L.pos.v[1] = 2. * d - 300.;
         bkg->L.pos.v[2] = ry;
         gltf_lightSet( bkg->L_idx, &bkg->L );
      }
   }
}

/**
 * @brief Creates a background Lua state from a script.
 */
static nlua_env *background_create( const char *name )
{
   size_t    bufsize;
   char      path[PATH_MAX];
   char     *buf;
   nlua_env *env;

   /* Create file name. */
   snprintf( path, sizeof( path ), BACKGROUND_PATH "%s.lua", name );

   /* Create the Lua env. */
   env = nlua_newEnv( name );
   nlua_loadStandard( env );
   nlua_loadTex( env );
   nlua_loadCol( env );
   nlua_loadBackground( env );
   nlua_loadCamera( env );
   nlua_loadGFX( env );

   /* Open file. */
   buf = ndata_read( path, &bufsize );
   if ( buf == NULL ) {
      WARN( _( "Background script '%s' not found." ), path );
      nlua_freeEnv( env );
      return NULL;
   }

   /* Load file. */
   if ( nlua_dobufenv( env, buf, bufsize, path ) != 0 ) {
      WARN( _( "Error loading background file: %s\n"
               "%s\n"
               "Most likely Lua file has improper syntax, please check" ),
            path, lua_tostring( naevL, -1 ) );
      free( buf );
      nlua_freeEnv( env );
      return NULL;
   }
   free( buf );

   return env;
}

/**
 * @brief Initializes the background system.
 */
int background_init( void )
{
   /* Load and set up the default Lua background state. */
   bkg_def_env = background_create( "default" );
   return 0;
}

/**
 * @brief Loads a background script by name.
 */
int background_load( const char *name )
{
   int       ret;
   nlua_env *env;

   NTracingZone( _ctx, 1 );

   /* Free if exists. */
   background_clear();

   /* Load default. */
   if ( name == NULL )
      bkg_cur_env = bkg_def_env;
   /* Load new script. */
   else
      bkg_cur_env = background_create( name );

   /* Comfort. */
   env = bkg_cur_env;
   if ( env == NULL ) {
      NTracingZoneEnd( _ctx );
      return -1;
   }

   /* Run Lua. */
   nlua_getenv( naevL, env, "background" );
   ret = nlua_pcall( env, 0, 0 );
   if ( ret != 0 ) { /* error has occurred */
      const char *err =
         ( lua_isstring( naevL, -1 ) ) ? lua_tostring( naevL, -1 ) : NULL;
      WARN( _( "Background -> 'background' : %s" ),
            ( err ) ? err : _( "unknown error" ) );
      lua_pop( naevL, 1 );
   }

   /* See if there are render functions. */
   bkg_L_renderbg = nlua_refenv( env, "renderbg" );
   bkg_L_rendermg = nlua_refenv( env, "rendermg" );
   bkg_L_renderfg = nlua_refenv( env, "renderfg" );
   bkg_L_renderov = nlua_refenv( env, "renderov" );

   NTracingZoneEnd( _ctx );

   return ret;
}

/**
 * @brief Destroys the current running background script.
 */
static void background_clearCurrent( void )
{
   if ( bkg_cur_env != bkg_def_env )
      nlua_freeEnv( bkg_cur_env );
   bkg_cur_env = NULL;

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
void background_clear( void )
{
   /* Destroy current background script. */
   background_clearCurrent();

   /* Clear the backgrounds. */
   background_clearImgArr( &bkg_image_arr_bk );
   background_clearImgArr( &bkg_image_arr_ft );

   /* Reset lighting. */
   gltf_lightReset();
}

/**
 * @brief Clears a background image array.
 *
 *    @param arr Array to clear.
 */
static void background_clearImgArr( background_image_t **arr )
{
   for ( int i = 0; i < array_size( *arr ); i++ ) {
      background_image_t *bkg = &( ( *arr )[i] );
      gl_freeTexture( bkg->image );
   }

   /* Erase it all. */
   array_erase( arr, array_begin( *arr ), array_end( *arr ) );
}

/**
 * @brief Cleans up and frees memory after the backgrounds.
 */
void background_free( void )
{
   /* Free the Lua. */
   background_clear();
   nlua_freeEnv( bkg_def_env );
   bkg_def_env = NULL;

   /* Free the images. */
   array_free( bkg_image_arr_ft );
   bkg_image_arr_ft = NULL;
   array_free( bkg_image_arr_bk );
   bkg_image_arr_bk = NULL;

   /* Free the Lua. */
   nlua_freeEnv( bkg_cur_env );
   bkg_cur_env = NULL;

   gl_vboDestroy( dust_vertexVBO );
   dust_vertexVBO = NULL;

   ndust = 0;
}

/**
 * @brief Returns an array (array.h) of star background images in the system
 * background.
 */
glTexture **background_getStarTextures( void )
{
   glTexture **imgs =
      array_create_size( glTexture *, array_size( bkg_image_arr_ft ) );
   for ( int i = 0; i < array_size( bkg_image_arr_ft ); i++ )
      array_push_back( &imgs, gl_dupTexture( bkg_image_arr_ft[i].image ) );
   return imgs;
}

/**
 * @brief Returns an overall background image (nebula, for instance), or NULL if
 * none exists.
 * @TODO With current background scripts, this only does anything on the border
 * (1 jump from nebula)!
 */
glTexture *background_getAmbientTexture( void )
{
   /* Assume many bg-layer images => none is representative => we should return
    * NULL. Example: Taiomi system's debris field. */
   if ( array_size( bkg_image_arr_bk ) == 1 )
      return gl_dupTexture( bkg_image_arr_bk[0].image );
   else
      return NULL;
}
