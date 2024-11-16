/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file ship.c
 *
 * @brief Handles the ship details.
 */
/** @cond */
#include "SDL_timer.h"
#include "physfs.h"

#include "naev.h"
/** @endcond */

#include "ship.h"

#include "array.h"
#include "colour.h"
#include "conf.h"
#include "faction.h"
#include "log.h"
#include "ndata.h"
#include "nlua.h"
#include "nlua_camera.h"
#include "nlua_gfx.h"
#include "nlua_ship.h"
#include "nstring.h"
#include "nxml.h"
#include "opengl_tex.h"
#include "shipstats.h"
#include "slots.h"
#include "sound.h"
#include "threadpool.h"

#define XML_SHIP "ship" /**< XML individual ship identifier. */

#define SHIP_ENGINE "_engine" /**< Engine graphic extension. */
#define SHIP_TARGET "_target" /**< Target graphic extension. */
#define SHIP_COMM "_comm"     /**< Communication graphic extension. */

#define VIEW_WIDTH 300  /**< Ship view window width. */
#define VIEW_HEIGHT 300 /**< Ship view window height. */

#define BUTTON_WIDTH 80  /**< Button width in ship view window. */
#define BUTTON_HEIGHT 30 /**< Button height in ship view window. */

#define STATS_DESC_MAX 512 /**< Maximum length for statistics description. */

/**
 * @brief Structure for threaded loading.
 */
typedef struct ShipThreadData_ {
   char *filename; /**< Filename. */
   Ship  ship;     /**< Ship data. */
   int   ret;      /**< Return status. */
} ShipThreadData;

static Ship *ship_stack = NULL; /**< Stack of ships available in the game. */

#define SHIP_FBO 3
static double       max_size            = 512.; /* Use at least 512 x 512. */
static double       ship_fbos           = 0.;
static GLuint       ship_fbo[SHIP_FBO]  = { GL_INVALID_ENUM };
static GLuint       ship_tex[SHIP_FBO]  = { GL_INVALID_ENUM };
static GLuint       ship_texd[SHIP_FBO] = { GL_INVALID_ENUM };
static const double ship_aa_scale_base  = 2.;
static double       ship_aa_scale       = -1.;

/*
 * Prototypes
 */
static int  ship_loadPLG( Ship *temp, const char *buf );
static int  ship_parse( Ship *temp, const char *filename, int firstpass );
static int  ship_parseThread( void *ptr );
static void ship_freeSlot( ShipOutfitSlot *s );
static void ship_renderFramebuffer3D( const Ship *s, GLuint fbo, double size,
                                      double fw, double fh, double engine_glow,
                                      double t, const glColour *c,
                                      const Lighting *L, const mat4 *H,
                                      int blit, unsigned int flags );

/**
 * @brief Compares two ship pointers for qsort.
 */
static int ship_cmp( const void *p1, const void *p2 )
{
   const Ship *s1 = p1;
   const Ship *s2 = p2;
   return strcmp( s1->name, s2->name );
}

/**
 * @brief Gets a ship based on its name.
 *
 *    @param name Name to match.
 *    @return Ship matching name or NULL if not found.
 */
const Ship *ship_get( const char *name )
{
   const Ship *s = ship_getW( name );
   if ( s == NULL )
      WARN( _( "Ship %s does not exist" ), name );
   return s;
}

/**
 * @brief Gets a ship based on its name without warning.
 *
 *    @param name Name to match.
 *    @return Ship matching name or NULL if not found.
 */
const Ship *ship_getW( const char *name )
{
   const Ship s = { .name = (char *)name };
   return bsearch( &s, ship_stack, array_size( ship_stack ), sizeof( Ship ),
                   ship_cmp );
}

/**
 * @brief Checks to see if an ship exists matching name (case insensitive).
 */
const char *ship_existsCase( const char *name )
{
   for ( int i = 0; i < array_size( ship_stack ); i++ )
      if ( strcasecmp( name, ship_stack[i].name ) == 0 )
         return ship_stack[i].name;
   return NULL;
}

/**
 * @brief Gets the array (array.h) of all ships.
 */
const Ship *ship_getAll( void )
{
   return ship_stack;
}

/**
 * @brief Comparison function for qsort().
 */
int ship_compareTech( const void *arg1, const void *arg2 )
{
   const Ship *s1, *s2;

   /* Get ships. */
   s1 = *(const Ship **)arg1;
   s2 = *(const Ship **)arg2;

   /* Compare rarity. */
   if ( s1->rarity < s2->rarity )
      return +1;
   else if ( s1->rarity > s2->rarity )
      return -1;

   /* Compare faction. */
   if ( s1->faction < s2->faction )
      return +1;
   else if ( s1->faction > s2->faction )
      return -1;

   /* Compare requirements. */
   /*
   if ( ( s1->condstr != NULL ) && ( s2->condstr == NULL ) )
      return -1;
   else if ( ( s2->condstr != NULL ) && ( s1->condstr == NULL ) )
      return +1;
   */

   /* Compare class. */
   if ( s1->class < s2->class )
      return +1;
   else if ( s1->class > s2->class )
      return -1;

   /* Compare price. */
   if ( s1->price < s2->price )
      return +1;
   else if ( s1->price > s2->price )
      return -1;

   /* Same. */
   return strcmp( s1->name, s2->name );
}

/**
 * @brief Gets the ship's class name in human readable form.
 *
 *    @param s Ship to get the class name from.
 *    @return The human readable class name.
 */
const char *ship_class( const Ship *s )
{
   return ship_classToString( s->class );
}

/**
 * @brief Gets the ship's display class in human readable form.
 *
 *    @param s Ship to get the display class name from.
 *    @return The human readable display class name.
 */
const char *ship_classDisplay( const Ship *s )
{
   if ( s->class_display )
      return s->class_display;
   return ship_class( s );
}

/**
 * @brief Gets the ship class name in human readable form.
 *
 *    @param class Class to get name of.
 *    @return The human readable class name.
 */
const char *ship_classToString( ShipClass class )
{
   switch ( class ) {
   case SHIP_CLASS_NULL:
      return "NULL";
   /* Civilian. */
   case SHIP_CLASS_YACHT:
      return N_( "Yacht" );
   case SHIP_CLASS_COURIER:
      return N_( "Courier" );
   case SHIP_CLASS_FREIGHTER:
      return N_( "Freighter" );
   case SHIP_CLASS_ARMOURED_TRANSPORT:
      return N_( "Armoured Transport" );
   case SHIP_CLASS_BULK_FREIGHTER:
      return N_( "Bulk Freighter" );
   /* Military. */
   case SHIP_CLASS_SCOUT:
      return N_( "Scout" );
   case SHIP_CLASS_INTERCEPTOR:
      return N_( "Interceptor" );
   case SHIP_CLASS_FIGHTER:
      return N_( "Fighter" );
   case SHIP_CLASS_BOMBER:
      return N_( "Bomber" );
   case SHIP_CLASS_CORVETTE:
      return N_( "Corvette" );
   case SHIP_CLASS_DESTROYER:
      return N_( "Destroyer" );
   case SHIP_CLASS_CRUISER:
      return N_( "Cruiser" );
   case SHIP_CLASS_BATTLESHIP:
      return N_( "Battleship" );
   case SHIP_CLASS_CARRIER:
      return N_( "Carrier" );
   /* Unknown. */
   default:
      return N_( "Unknown" );
   }
}

#define STRTOSHIP( x, y )                                                      \
   if ( strcmp( str, x ) == 0 )                                                \
   return y
/**
 * @brief Gets the machine ship class identifier from a human readable string.
 *
 *    @param str String to extract ship class identifier from.
 */
ShipClass ship_classFromString( const char *str )
{
   if ( str == NULL )
      return SHIP_CLASS_NULL;
   /* Civilian */
   STRTOSHIP( "Yacht", SHIP_CLASS_YACHT );
   STRTOSHIP( "Courier", SHIP_CLASS_COURIER );
   STRTOSHIP( "Freighter", SHIP_CLASS_FREIGHTER );
   STRTOSHIP( "Armoured Transport", SHIP_CLASS_ARMOURED_TRANSPORT );
   STRTOSHIP( "Bulk Freighter", SHIP_CLASS_BULK_FREIGHTER );

   /* Military */
   STRTOSHIP( "Scout", SHIP_CLASS_SCOUT );
   STRTOSHIP( "Interceptor", SHIP_CLASS_INTERCEPTOR );
   STRTOSHIP( "Fighter", SHIP_CLASS_FIGHTER );
   STRTOSHIP( "Bomber", SHIP_CLASS_BOMBER );
   STRTOSHIP( "Corvette", SHIP_CLASS_CORVETTE );
   STRTOSHIP( "Destroyer", SHIP_CLASS_DESTROYER );
   STRTOSHIP( "Cruiser", SHIP_CLASS_CRUISER );
   STRTOSHIP( "Battleship", SHIP_CLASS_BATTLESHIP );
   STRTOSHIP( "Carrier", SHIP_CLASS_CARRIER );

   /* Unknown */
   return SHIP_CLASS_NULL;
}
#undef STRTOSHIP

/**
 * @brief Gets the ship's base price (no outfits).
 */
credits_t ship_basePrice( const Ship *s )
{
   return s->price;
}

/**
 * @brief The ship buy price, includes default outfits.
 */
credits_t ship_buyPrice( const Ship *s )
{
   /* Get base price. */
   credits_t price = ship_basePrice( s );

   for ( int i = 0; i < array_size( s->outfit_structure ); i++ ) {
      const Outfit *o = s->outfit_structure[i].data;
      if ( o != NULL )
         price += o->price;
   }
   for ( int i = 0; i < array_size( s->outfit_utility ); i++ ) {
      const Outfit *o = s->outfit_utility[i].data;
      if ( o != NULL )
         price += o->price;
   }
   for ( int i = 0; i < array_size( s->outfit_weapon ); i++ ) {
      const Outfit *o = s->outfit_weapon[i].data;
      if ( o != NULL )
         price += o->price;
   }

   return price;
}

void ship_renderGfxStore( GLuint fbo, const Ship *s, int size, double dir,
                          double updown, double glow )
{
   glBindFramebuffer( GL_FRAMEBUFFER, fbo );
   glClear( GL_COLOR_BUFFER_BIT );

   /* Give faction colour background if applicable. */
   if ( s->faction >= 0 ) {
      const glColour *c = faction_colour( s->faction );
      if ( c != NULL ) {
         glUseProgram( shaders.shop_bg.program );
         gl_renderShader( 0., 0., size, size, 0., &shaders.shop_bg, c, 0 );
      }
   }

   if ( s->gfx_3d != NULL ) {
      Lighting L = L_store_const;
      mat4     H;
      double   t = SDL_GetTicks() / 1000.;

      /* We rotate the model so it's staring at the player and facing slightly
       * down. */
      H = mat4_identity();
      mat4_rotate( &H, -M_PI_4 + dir, 0.0, 1.0, 0.0 );
      mat4_rotate( &H, -M_PI_4 * 0.25 + updown, 1.0, 0.0, 0.0 );

      /* Render the model. */
      ship_renderFramebuffer3D( s, fbo, size, gl_screen.nw, gl_screen.nh, glow,
                                t, &cWhite, &L, &H, 0, OPENGL_TEX_VFLIP );
      /* Already restore current framebuffer. */
   } else if ( s->gfx_comm != NULL ) {
      glTexture *glcomm;
      double     scale, w, h, tx, ty;
      int        sx, sy;

      glcomm = gl_newImage( s->gfx_comm, 0 );
      glcomm->flags &= ~OPENGL_TEX_VFLIP;

      scale = MIN( size / glcomm->w, size / glcomm->h );
      w     = scale * glcomm->w;
      h     = scale * glcomm->h;
      gl_getSpriteFromDir( &sx, &sy, s->sx, s->sy, dir );
      tx = glcomm->sw * (double)( sx ) / glcomm->w;
      ty = glcomm->sh * ( glcomm->sy - (double)sy - 1 ) / glcomm->h;
      gl_renderTexture( glcomm, ( size - w ) * 0.5, ( size - h ) * 0.5, w, h,
                        tx, ty, glcomm->srw, glcomm->srh, NULL, 0. );

      gl_freeTexture( glcomm );

      glBindFramebuffer( GL_FRAMEBUFFER, gl_screen.current_fbo );
   }
}

/**
 * @brief Get the store gfx.
 */
glTexture *ship_gfxStore( const Ship *s, int size, double dir, double updown,
                          double glow )
{
   double     fbosize;
   GLuint     fbo, tex;
   char       buf[STRMAX_SHORT];
   glTexture *gltex;

   /* Load graphic if applicable. */
   ship_gfxLoad( (Ship *)s );

   fbosize = size / gl_screen.scale;

   gl_contextSet();
   gl_fboCreate( &fbo, &tex, fbosize, fbosize );

   ship_renderGfxStore( fbo, s, size, dir, updown, glow );

   glDeleteFramebuffers( 1, &fbo ); /* No need for FBO. */
   glBindFramebuffer( GL_FRAMEBUFFER, gl_screen.current_fbo );
   gl_checkErr();
   gl_contextUnset();

   snprintf( buf, sizeof( buf ), "%s_fbo_gfx_store_%d", s->name, size );
   gltex = gl_rawTexture( buf, tex, fbosize, fbosize );
   gltex->flags |= OPENGL_TEX_VFLIP;

   return gltex;
}

/**
 * @brief Loads the ship's comm graphic.
 *
 * Must be freed afterwards.
 */
glTexture *ship_gfxComm( const Ship *s, int size, double tilt, double dir,
                         const Lighting *Lscene )
{
   double     fbosize;
   GLuint     fbo, tex, depth_tex;
   char       buf[STRMAX_SHORT];
   glTexture *gltex;

   fbosize = ceil( size / gl_screen.scale );

   /* Load graphic if applicable. */
   ship_gfxLoad( (Ship *)s );

   gl_contextSet();
   gl_fboCreate( &fbo, &tex, fbosize, fbosize );
   gl_fboAddDepth( fbo, &depth_tex, fbosize, fbosize );
   glBindFramebuffer( GL_FRAMEBUFFER, fbo );
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   if ( s->gfx_3d != NULL ) {
      Lighting L = ( Lscene == NULL ) ? L_default : *Lscene;
      mat4     H, Hlight;
      double   t = SDL_GetTicks() / 1000.;
      double   rendersize =
         MIN( MIN( ceil( ship_aa_scale_base * fbosize ), gl_screen.rw ),
              gl_screen.rh );
      snprintf( buf, sizeof( buf ), "%s_fbo_gfx_comm_%d", s->name, size );

      /* We rotate the model so it's staring at the player and facing slightly
       * down. */
      H = mat4_identity();
      mat4_rotate( &H, -M_PI_4, 0.0, 1.0, 0.0 );
      mat4_rotate( &H, -M_PI_4 * 0.25, 1.0, 0.0, 0.0 );

      /* Transform the light so it's consistent with the 3D model. */
      Hlight = mat4_identity();
      if ( fabs( tilt ) > DOUBLE_TOL ) {
         mat4_rotate( &Hlight, M_PI_2, 0.0, 1.0, 0.0 );
         mat4_rotate( &Hlight, -tilt, 1.0, 0.0, 0.0 );
         mat4_rotate( &Hlight, dir, 0.0, 1.0, 0.0 );
      } else
         mat4_rotate( &Hlight, dir + M_PI_2, 0.0, 1.0, 0.0 );
      gltf_lightTransform( &L, &Hlight );

      /* Increase ambient a wee bit. */
      L.ambient_r += 0.1;
      L.ambient_g += 0.1;
      L.ambient_b += 0.1;
      L.intensity *= 1.5;

      /* Render the model. */
      glBindFramebuffer( GL_FRAMEBUFFER, gl_screen.fbo[2] );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      gltf_renderScene( gl_screen.fbo[2], s->gfx_3d, s->gfx_3d->scene_body, &H,
                        t, rendersize, &L );
      glBindFramebuffer( GL_READ_FRAMEBUFFER, gl_screen.fbo[2] );
      glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fbo );
      glBlitFramebuffer( 0, 0, rendersize, rendersize, 0, fbosize, fbosize, 0,
                         GL_COLOR_BUFFER_BIT, GL_LINEAR );
   } else if ( s->gfx_comm != NULL ) {
      glTexture *glcomm;
      double     scale, w, h;

      snprintf( buf, sizeof( buf ), "%s_fbo_gfx_comm_%d", s->gfx_comm, size );
      glcomm = gl_newImage( s->gfx_comm, 0 );
      glcomm->flags &= ~OPENGL_TEX_VFLIP;

      scale = MIN( size / glcomm->w, size / glcomm->h );
      w     = scale * glcomm->w;
      h     = scale * glcomm->h;
      gl_renderTexture( glcomm, ( size - w ) * 0.5, ( size - h ) * 0.5, w, h,
                        0., 0., 1., 1., NULL, 0. );

      gl_freeTexture( glcomm );
   } else
      WARN( _( "Unable to render comm graphic for ship '%s'!" ), s->name );

   glDeleteFramebuffers( 1, &fbo ); /* No need for FBO. */
   glDeleteTextures( 1, &depth_tex );
   glBindFramebuffer( GL_FRAMEBUFFER, gl_screen.current_fbo );
   gl_checkErr();
   gl_contextUnset();

   gltex = gl_rawTexture( buf, tex, fbosize, fbosize );
   gltex->flags |= OPENGL_TEX_VFLIP;

   return gltex;
}

/**
 * @brief Returns whether or not the ship has animated graphics.
 *
 *    @param s Ship to check to see if has animated graphics.
 *    @return Whether or not a ship has animated graphics.
 */
int ship_gfxAnimated( const Ship *s )
{
   if ( s->gfx_3d == NULL )
      return 0;
   return ( s->gfx_3d->nanimations > 0 );
}

/**
 * @brief Gets the size of the ship.
 *
 *    @brief s Ship to get the size of.
 *    @return Size of the ship.
 */
int ship_size( const Ship *s )
{
   switch ( s->class ) {
   case SHIP_CLASS_YACHT:
   case SHIP_CLASS_SCOUT:
   case SHIP_CLASS_INTERCEPTOR:
      return 1;

   case SHIP_CLASS_COURIER:
   case SHIP_CLASS_FIGHTER:
   case SHIP_CLASS_BOMBER:
      return 2;

   case SHIP_CLASS_FREIGHTER:
   case SHIP_CLASS_CORVETTE:
      return 3;

   case SHIP_CLASS_DESTROYER:
   case SHIP_CLASS_ARMOURED_TRANSPORT:
      return 4;

   case SHIP_CLASS_BULK_FREIGHTER:
   case SHIP_CLASS_CRUISER:
      return 5;

   case SHIP_CLASS_BATTLESHIP:
   case SHIP_CLASS_CARRIER:
      return 6;

   default:
      return -1;
   }
}

/**
 * @brief Loads the space graphics for a ship from an image.
 *
 *    @param temp Ship to load into.
 *    @param str Path of the image to use.
 *    @param sx Number of X sprites in image.
 *    @param sy Number of Y sprites in image.
 */
static int ship_loadSpaceImage( Ship *temp, const char *str, int sx, int sy )
{
   unsigned int flags = OPENGL_TEX_MIPMAPS | OPENGL_TEX_VFLIP;
   /* If no collision polygon, we use transparency mapping. */
   if ( array_size( temp->polygon.views ) <= 0 )
      flags |= OPENGL_TEX_MAPTRANS;
   temp->gfx_space = gl_newSprite( str, sx, sy, flags );
   return 0;
}

/**
 * @brief Loads the space graphics for a ship from an image.
 *
 *    @param temp Ship to load into.
 *    @param str Path of the image to use.
 *    @param sx Number of X sprites in image.
 *    @param sy Number of Y sprites in image.
 */
static int ship_loadEngineImage( Ship *temp, const char *str, int sx, int sy )
{
   temp->gfx_engine = gl_newSprite( str, sx, sy, OPENGL_TEX_MIPMAPS );
   return ( temp->gfx_engine != NULL );
}

/**
 * @brief Checks to see if a ship has loaded graphics.
 */
int ship_gfxLoaded( const Ship *s )
{
   return ( ( s->gfx_3d != NULL ) || ( s->gfx_space != NULL ) );
}

/**
 * @brief Tries to load the graphics for all ships that need it.
 */
int ship_gfxLoadNeeded( void )
{
   ThreadQueue *tq = vpool_create();
   SDL_GL_MakeCurrent( gl_screen.window, NULL );

   for ( int i = 0; i < array_size( ship_stack ); i++ ) {
      Ship *s = &ship_stack[i];
      if ( !ship_isFlag( s, SHIP_NEEDSGFX ) )
         continue;
      vpool_enqueue( tq, (int ( * )( void * ))ship_gfxLoad, s );
      ship_rmFlag( s, SHIP_NEEDSGFX );
   }

   vpool_wait( tq );
   vpool_cleanup( tq );

   SDL_GL_MakeCurrent( gl_screen.window, gl_screen.context );
   return 0;
}

/**
 * @brief Loads the graphics for a ship if necessary.
 *
 *    @param s Ship to load into.
 */
int ship_gfxLoad( Ship *s )
{
   char        str[PATH_MAX], *base;
   const char *delim, *base_path;
   const char *ext    = ".webp";
   const char *buf    = s->gfx_path;
   int         sx     = s->sx;
   int         sy     = s->sy;
   int         engine = !s->noengine;

   /* If already loaded, just ignore. */
   if ( ship_gfxLoaded( s ) )
      return 0;

   /* Get base path. */
   delim     = strchr( buf, '_' );
   base      = delim == NULL ? strdup( buf ) : strndup( buf, delim - buf );
   base_path = ( s->base_path != NULL ) ? s->base_path : s->base_type;

   /* Load the 3d model */
   snprintf( str, sizeof( str ), SHIP_3DGFX_PATH "%s/%s.gltf", base_path, buf );
   if ( PHYSFS_exists( str ) ) {
      // DEBUG( "Found 3D graphics for '%s' at '%s'!", s->name, str );
      s->gfx_3d = gltf_loadFromFile( str );

      /* Replace trails if applicable. */
      if ( array_size( s->gfx_3d->trails ) > 0 ) {
         array_erase( &s->trail_emitters, array_begin( s->trail_emitters ),
                      array_end( s->trail_emitters ) );
         ship_setFlag( s, SHIP_3DTRAILS );

         for ( int i = 0; i < array_size( s->gfx_3d->trails ); i++ ) {
            GltfTrail       *t = &s->gfx_3d->trails[i];
            ShipTrailEmitter trail;

            /* New trail. */
            trail.pos = t->pos;
            vec3_scale( &trail.pos, s->size * 0.5 ); /* Convert to "pixels" */
            trail.trail_spec = trailSpec_get( t->generator );
            trail.flags      = 0;

            if ( trail.trail_spec != NULL )
               array_push_back( &s->trail_emitters, trail );
         }
      }

      /* Replace mount points if applicable. */
      if ( array_size( s->gfx_3d->mounts ) > 0 ) {
         int n = array_size( s->gfx_3d->mounts );

         ship_setFlag( s, SHIP_3DMOUNTS );
         if ( n < array_size( s->outfit_weapon ) )
            WARN( _( "Number of 3D weapon mounts from GLTF file for ship '%s' "
                     "is less than the "
                     "number of ship weapons! Got %d, expected at least %d." ),
                  s->name, n, array_size( s->outfit_weapon ) );

         for ( int i = 0; i < array_size( s->outfit_weapon ); i++ ) {
            ShipMount *sm = &s->outfit_weapon[i].mount;
            vec3_copy( &sm->pos,
                       &s->gfx_3d->mounts[i % n].pos ); /* Loop over. */
            vec3_scale( &sm->pos, s->size * 0.5 ); /* Convert to "pixels" */
         }
      }
   }

   /* Determine extension path. */
   if ( buf[0] == '/' ) /* absolute path. */
      snprintf( str, sizeof( str ), "%s", buf );
   else {
      snprintf( str, sizeof( str ), SHIP_GFX_PATH "%s/%s%s", base, buf, ext );
      if ( !PHYSFS_exists( str ) ) {
         ext = ".png";
         snprintf( str, sizeof( str ), SHIP_GFX_PATH "%s/%s%s", base, buf,
                   ext );
      }
   }

   /* Load the polygon. */
   ship_loadPLG( s,
                 ( s->polygon_path != NULL ) ? s->polygon_path : s->gfx_path );

   /* If we have 3D and polygons, we'll ignore the 2D stuff. */
   if ( ( s->gfx_3d != NULL ) && ( array_size( s->polygon.views ) > 0 ) ) {
      free( base );
      return 0;
   }

   /* Get the comm graphic for future loading. */
   if ( s->gfx_comm == NULL )
      SDL_asprintf( &s->gfx_comm, SHIP_GFX_PATH "%s/%s" SHIP_COMM "%s", base,
                    buf, ext );

   /* Load the space sprite. */
   ship_loadSpaceImage( s, str, sx, sy );

   /* Load the engine sprite .*/
   if ( engine ) {
      snprintf( str, sizeof( str ), SHIP_GFX_PATH "%s/%s" SHIP_ENGINE "%s",
                base, buf, ext );
      ship_loadEngineImage( s, str, sx, sy );
      if ( s->gfx_engine == NULL )
         WARN( _( "Ship '%s' does not have an engine sprite (%s)." ), s->name,
               str );
   }
   free( base );

#if 0
#if DEBUGGING
   if ( ( s->gfx_space != NULL ) &&
        ( round( s->size ) != round( s->gfx_space->sw ) ) )
      WARN( ( "Mismatch between 'size' and 'gfx_space' sprite size for ship "
              "'%s'! 'size' should be %.0f!" ),
            s->name, s->gfx_space->sw );
#endif /* DEBUGGING */
#endif

   return 0;
}

/**
 * @brief Loads the collision polygon for a ship.
 *
 *    @param temp Ship to load into.
 *    @param buf Name of the file.
 */
static int ship_loadPLG( Ship *temp, const char *buf )
{
   char       file[PATH_MAX];
   xmlDocPtr  doc;
   xmlNodePtr node;

   if ( temp->gfx_3d != NULL )
      snprintf( file, sizeof( file ), "%s%s.xml", SHIP_POLYGON_PATH3D, buf );
   else
      snprintf( file, sizeof( file ), "%s%s.xml", SHIP_POLYGON_PATH, buf );

   /* See if the file does exist. */
   if ( !PHYSFS_exists( file ) ) {
      WARN( _( "%s xml collision polygon does not exist! Please use the "
               "script '%s' found in Naev's main repository." ),
            file, "utils/polygonize.py" );
      return 0;
   }

   /* Load the XML. */
   doc = xml_parsePhysFS( file );
   if ( doc == NULL )
      return 0;

   node = doc->xmlChildrenNode; /* First polygon node */
   if ( node == NULL ) {
      xmlFreeDoc( doc );
      WARN( _( "Malformed %s file: does not contain elements" ), file );
      return 0;
   }

   do { /* load the polygon data */
      if ( xml_isNode( node, "polygons" ) )
         poly_load( &temp->polygon, node );
   } while ( xml_nextNode( node ) );

   xmlFreeDoc( doc );
   return 0;
}

/**
 * @brief Parses a slot for a ship.
 *
 *    @param temp Ship to be parsed.
 *    @param slot Slot being parsed.
 *    @param type Type of the slot.
 *    @param node Node containing the data.
 *    @return 0 on success.
 */
static int ship_parseSlot( Ship *temp, ShipOutfitSlot *slot,
                           OutfitSlotType type, xmlNodePtr node )
{
   OutfitSlotSize base_size;
   char          *buf;

   /* Initialize. */
   memset( slot, 0, sizeof( ShipOutfitSlot ) );
   /* Parse size. */
   xmlr_attr_strd( node, "size", buf );
   if ( buf != NULL )
      base_size = outfit_toSlotSize( buf );
   else {
      WARN( _( "Ship '%s' has undefined slot size, setting to '%s'" ),
            temp->name, "Small" );
      base_size = OUTFIT_SLOT_SIZE_LIGHT;
   }
   free( buf );

   /* Get mount point for weapons. */
   if ( type == OUTFIT_SLOT_WEAPON ) {
      xmlr_attr_float( node, "x", slot->mount.pos.v[0] );
      xmlr_attr_float( node, "y", slot->mount.pos.v[1] );
      /* Since we measure in pixels, we have to modify it so it
       *  doesn't get corrected by the ortho correction. */
      slot->mount.pos.v[1] *= M_SQRT2;
      xmlr_attr_float( node, "h", slot->mount.pos.v[2] );
   }

   /* Parse property. */
   xmlr_attr_strd( node, "prop", buf );
   if ( buf != NULL ) {
      slot->slot.spid = sp_get( buf );
      slot->exclusive = sp_exclusive( slot->slot.spid );
      slot->required  = sp_required( slot->slot.spid );
      slot->visible   = sp_visible( slot->slot.spid );
      slot->locked    = sp_locked( slot->slot.spid );
      free( buf );
   }
   // TODO: consider inserting those two parse blocks below inside the parse
   // block above

   /* Parse exclusive flag, default false. */
   xmlr_attr_int_def( node, "exclusive", slot->exclusive, slot->exclusive );
   /* TODO: decide if exclusive should even belong in ShipOutfitSlot,
    * remove this hack, and fix slot->exclusive to slot->slot.exclusive
    * in its two previous occurrences, meaning three lines above and 12
    * lines above */
   /* hack */
   slot->slot.exclusive = slot->exclusive;

   /* Parse required flag, default false. */
   xmlr_attr_int_def( node, "required", slot->required, slot->required );

   /* Parse locked flag, default false. */
   xmlr_attr_int_def( node, "locked", slot->locked, slot->locked );

   /* Name if applicable. */
   xmlr_attr_strd( node, "name", slot->name );

   /* Parse default outfit. */
   buf = xml_get( node );
   if ( buf != NULL ) {
      const Outfit *o = outfit_get( buf );
      if ( o == NULL )
         WARN( _( "Ship '%s' has default outfit '%s' which does not exist." ),
               temp->name, buf );
      slot->data = o;
   }

   /* Set stuff. */
   slot->slot.size = base_size;
   slot->slot.type = type;

   /* Required slots need a default outfit. */
   if ( slot->required && ( slot->data == NULL ) )
      WARN( _( "Ship '%s' has required slot without a default outfit." ),
            temp->name );

   return 0;
}

/**
 * @brief Extracts the in-game ship from an XML node.
 *
 *    @param temp Ship to load data into.
 *    @param filename File to load ship from.
 *    @param firstpass Whether or not on the first pass.
 *    @return 0 on success.
 */
static int ship_parse( Ship *temp, const char *filename, int firstpass )
{
   xmlNodePtr       parent, node;
   xmlDocPtr        doc;
   ShipTrailEmitter trail;

   /* On second pass we ignore inheriting ones. */
   if ( ( !firstpass ) && ( temp->inherits == NULL ) )
      return 0;

   if ( firstpass ) {
      /* Load the XML. */
      doc = xml_parsePhysFS( filename );
      if ( doc == NULL )
         return -1;

      parent = doc->xmlChildrenNode; /* First ship node */
      if ( parent == NULL ) {
         xmlFreeDoc( doc );
         WARN( _( "Malformed %s file: does not contain elements" ), filename );
         return -1;
      }

      /* Clear memory. */
      memset( temp, 0, sizeof( Ship ) );

      /* Defaults. */
      ss_statsInit( &temp->stats_array );
      temp->dt_default     = 1.;
      temp->faction        = -1;
      temp->trail_emitters = array_create( ShipTrailEmitter );

      /* Lua defaults. */
      temp->lua_env            = LUA_NOREF;
      temp->lua_descextra      = LUA_NOREF;
      temp->lua_init           = LUA_NOREF;
      temp->lua_cleanup        = LUA_NOREF;
      temp->lua_update         = LUA_NOREF;
      temp->lua_dt             = 0.1;
      temp->lua_explode_init   = LUA_NOREF;
      temp->lua_explode_update = LUA_NOREF;

      /* Get name. */
      xmlr_attr_strd_free( parent, "name", temp->name );
      if ( temp->name == NULL )
         WARN( _( "Ship in %s has invalid or no name" ), SHIP_DATA_PATH );

      /* Get inheritance. */
      xmlr_attr_strd_free( parent, "inherits", temp->inherits );
      if ( temp->inherits != NULL ) {
         /* Don't free doc as it gets reused next iteration. */
         temp->rawdata = doc;
         return 0;
      }
   } else {
      doc    = temp->rawdata;
      parent = doc->xmlChildrenNode; /* First ship node */
#define STRDUP_( x ) ( ( x == NULL ) ? NULL : strdup( x ) )
#define ARRAYDUP_( x, y )                                                      \
   do {                                                                        \
      for ( int i = 0; i < array_size( y ); i++ )                              \
         array_push_back( &x, y[i] );                                          \
   } while ( 0 )
      Ship  t             = *temp;
      Ship *base          = (Ship *)ship_get( temp->inherits );
      *temp               = *base;
      temp->rawdata       = doc;
      temp->inherits      = t.inherits;
      temp->name          = t.name;
      temp->base_type     = STRDUP_( base->base_type );
      temp->base_path     = STRDUP_( base->base_path );
      temp->class_display = STRDUP_( base->class_display );
      temp->license       = STRDUP_( base->license );
      temp->cond          = STRDUP_( base->cond );
      temp->condstr       = STRDUP_( base->condstr );
      temp->fabricator    = STRDUP_( base->fabricator );
      temp->description   = STRDUP_( base->description );
      temp->desc_extra    = STRDUP_( base->desc_extra );
      temp->gfx_path      = STRDUP_( base->gfx_path );
      temp->polygon_path  = STRDUP_( base->polygon_path );
      temp->gfx_comm      = STRDUP_( base->gfx_comm );
      if ( base->gfx_overlays != NULL ) {
         temp->gfx_overlays = array_create( glTexture * );
         for ( int i = 0; i < array_size( base->gfx_overlays ); i++ )
            array_push_back( &temp->gfx_overlays,
                             gl_dupTexture( base->gfx_overlays[i] ) );
      }
      temp->trail_emitters   = t.trail_emitters;
      temp->outfit_structure = array_create( ShipOutfitSlot );
      ARRAYDUP_( temp->outfit_structure, base->outfit_structure );
      temp->outfit_utility = array_create( ShipOutfitSlot );
      ARRAYDUP_( temp->outfit_utility, base->outfit_utility );
      temp->outfit_weapon = array_create( ShipOutfitSlot );
      ARRAYDUP_( temp->outfit_weapon, base->outfit_weapon );
      temp->desc_stats = STRDUP_( base->desc_stats );
      temp->stats      = NULL;
      ShipStatList *ll = base->stats;
      while ( ll != NULL ) {
         ShipStatList *ln = calloc( 1, sizeof( ShipStatList ) );
         *ln              = *ll;
         ln->next         = temp->stats;
         temp->stats      = ln;
         ll               = ll->next;
      }
      temp->tags = array_create( char * );
      for ( int i = 0; i < array_size( base->tags ); i++ )
         array_push_back( &temp->tags, strdup( base->tags[i] ) );
      temp->lua_file = STRDUP_( base->lua_file );
#undef STRDUP_

      /* Have to correct some post-processing. */
      temp->dmg_absorb *= 100.;
      temp->turn *= 180. / M_PI;
   }

   /* Load the rest of the data. */
   node = parent->xmlChildrenNode;
   do { /* load all the data */

      /* Only handle nodes. */
      xml_onlyNodes( node );

      if ( xml_isNode( node, "class" ) ) {
         xmlr_attr_strd_free( node, "display", temp->class_display );
         temp->class = ship_classFromString( xml_get( node ) );
         continue;
      }
      if ( xml_isNode( node, "gfx" ) || xml_isNode( node, "GFX" ) ) {
         const char *str = xml_get( node );
         /* TODO remove for 0.13.0 */
         if ( xml_isNode( node, "GFX" ) )
            WARN( _( "Ship '%s': using <GFX> instead of <gfx>!" ), temp->name );

         /* Get base graphic name. */
         if ( str == NULL ) {
            WARN( _( "Ship '%s' has NULL tag '%s'!" ), temp->name, "gfx" );
            continue;
         } else {
            free( temp->gfx_path );
            temp->gfx_path = strdup( str );
         }

         /* Parse attributes. */
         xmlr_attr_float_def( node, "size", temp->size, 1 );
         xmlr_attr_int_def( node, "sx", temp->sx, 8 );
         xmlr_attr_int_def( node, "sy", temp->sy, 8 );
         xmlr_attr_strd_free( node, "comm", temp->gfx_comm );
         xmlr_attr_int( node, "noengine", temp->noengine );
         xmlr_attr_strd_free( node, "polygon", temp->polygon_path );

         /* Graphics are now lazy loaded. */

         continue;
      }

      if ( xml_isNode( node, "faction" ) ) {
         temp->faction = faction_get( xml_get( node ) );
         continue;
      }

      if ( xml_isNode( node, "gfx_overlays" ) ) {
         xmlNodePtr cur = node->children;
         array_free( temp->gfx_overlays );
         temp->gfx_overlays = array_create_size( glTexture *, 2 );
         do {
            xml_onlyNodes( cur );
            if ( xml_isNode( cur, "gfx_overlay" ) )
               array_push_back( &temp->gfx_overlays,
                                xml_parseTexture( cur, OVERLAY_GFX_PATH "%s", 1,
                                                  1, OPENGL_TEX_MIPMAPS ) );
         } while ( xml_nextNode( cur ) );
         continue;
      }

      if ( xml_isNode( node, "sound" ) ) {
         xmlr_attr_float_def( node, "pitch", temp->engine_pitch, 1. );
         temp->sound = sound_get( xml_get( node ) );
         continue;
      }
      if ( xml_isNode( node, "base_type" ) ) {
         const char *nstr = xml_get( node );
         xmlr_attr_strd_free( node, "path", temp->base_path );
         free( temp->base_type );
         temp->base_type = ( nstr != NULL ) ? strdup( nstr ) : NULL;
         continue;
      }
      xmlr_float( node, "time_mod", temp->dt_default );
      xmlr_long( node, "price", temp->price );
      xmlr_strd_free( node, "license", temp->license );
      xmlr_strd_free( node, "cond", temp->cond );
      xmlr_strd_free( node, "condstr", temp->condstr );
      xmlr_strd_free( node, "fabricator", temp->fabricator );
      xmlr_strd_free( node, "description", temp->description );
      xmlr_strd_free( node, "desc_extra", temp->desc_extra );
      xmlr_int( node, "points", temp->points );
      xmlr_int( node, "rarity", temp->rarity );
      if ( xml_isNode( node, "lua" ) ) {
         const char *nstr = xml_get( node );
         if ( nstr == NULL ) {
            WARN( _( "Ship '%s' has invalid '%s' node." ), temp->name, "lua" );
            continue;
         }
         free( temp->lua_file );
         if ( nstr[0] == '/' )
            temp->lua_file = strdup( nstr );
         else
            SDL_asprintf( &temp->lua_file, SHIP_DATA_LUA_PATH "%s", nstr );
         continue;
      }

      if ( xml_isNode( node, "flags" ) ) {
         xmlNodePtr cur = node->children;
         do {
            xml_onlyNodes( cur );
            if ( xml_isNode( cur, "noplayer" ) ) {
               ship_setFlag( temp, SHIP_NOPLAYER );
               continue;
            }
            if ( xml_isNode( cur, "noescort" ) ) {
               ship_setFlag( temp, SHIP_NOESCORT );
               continue;
            }
            if ( xml_isNode( cur, "unique" ) ) {
               ship_setFlag( temp, SHIP_UNIQUE );
               continue;
            }
            // cppcheck-suppress nullPointerRedundantCheck
            WARN( _( "Ship '%s' has unknown flags node '%s'." ), temp->name,
                  cur->name );
         } while ( xml_nextNode( cur ) );
         continue;
      }

      if ( xml_isNode( node, "trail_generator" ) ) {
         const char *buf;
         xmlr_attr_float( node, "x", trail.pos.v[0] );
         xmlr_attr_float( node, "y", trail.pos.v[1] );
         xmlr_attr_float( node, "h", trail.pos.v[2] );
         xmlr_attr_int_def( node, "always_under", trail.flags,
                            SHIP_TRAIL_ALWAYS_UNDER );
         buf = xml_get( node );
         if ( buf == NULL )
            buf = "default";
         trail.trail_spec = trailSpec_get( buf );
         if ( trail.trail_spec != NULL )
            array_push_back( &temp->trail_emitters, trail );
         continue;
      }

      if ( xml_isNode( node, "movement" ) ) {
         xmlNodePtr cur = node->children;
         do {
            xml_onlyNodes( cur );
            xmlr_float( cur, "accel", temp->accel );
            xmlr_float( cur, "turn", temp->turn );
            xmlr_float( cur, "speed", temp->speed );
            /* All the xmlr_ stuff have continue cases. */
            // cppcheck-suppress nullPointerRedundantCheck
            WARN( _( "Ship '%s' has unknown movement node '%s'." ), temp->name,
                  cur->name );
         } while ( xml_nextNode( cur ) );
         continue;
      }
      if ( xml_isNode( node, "health" ) ) {
         xmlNodePtr cur = node->children;
         do {
            xml_onlyNodes( cur );
            xmlr_float( cur, "absorb", temp->dmg_absorb );
            xmlr_float( cur, "armour", temp->armour );
            xmlr_float( cur, "armour_regen", temp->armour_regen );
            xmlr_float( cur, "shield", temp->shield );
            xmlr_float( cur, "shield_regen", temp->shield_regen );
            xmlr_float( cur, "energy", temp->energy );
            xmlr_float( cur, "energy_regen", temp->energy_regen );
            /* All the xmlr_ stuff have continue cases. */
            // cppcheck-suppress nullPointerRedundantCheck
            WARN( _( "Ship '%s' has unknown health node '%s'." ), temp->name,
                  cur->name );
         } while ( xml_nextNode( cur ) );
         continue;
      }
      if ( xml_isNode( node, "characteristics" ) ) {
         xmlNodePtr cur = node->children;
         do {
            xml_onlyNodes( cur );
            xmlr_int( cur, "crew", temp->crew );
            xmlr_float( cur, "mass", temp->mass );
            xmlr_float( cur, "cpu", temp->cpu );
            xmlr_int( cur, "fuel", temp->fuel );
            xmlr_int( cur, "fuel_consumption", temp->fuel_consumption );
            xmlr_float( cur, "cargo", temp->cap_cargo );
            /* All the xmlr_ stuff have continue cases. */
            // cppcheck-suppress nullPointerRedundantCheck
            WARN( _( "Ship '%s' has unknown characteristic node '%s'." ),
                  temp->name, cur->name );
         } while ( xml_nextNode( cur ) );
         continue;
      }
      if ( xml_isNode( node, "slots" ) ) {
         /* Clean up, just in case. */
         array_free( temp->outfit_structure );
         array_free( temp->outfit_utility );
         array_free( temp->outfit_weapon );
         /* Allocate the space. */
         temp->outfit_structure = array_create( ShipOutfitSlot );
         temp->outfit_utility   = array_create( ShipOutfitSlot );
         temp->outfit_weapon    = array_create( ShipOutfitSlot );

         /* Initialize the mounts. */
         xmlNodePtr cur = node->children;
         do {
            xml_onlyNodes( cur );
            if ( xml_isNode( cur, "structure" ) )
               ship_parseSlot( temp, &array_grow( &temp->outfit_structure ),
                               OUTFIT_SLOT_STRUCTURE, cur );
            else if ( xml_isNode( cur, "utility" ) )
               ship_parseSlot( temp, &array_grow( &temp->outfit_utility ),
                               OUTFIT_SLOT_UTILITY, cur );
            else if ( xml_isNode( cur, "weapon" ) )
               ship_parseSlot( temp, &array_grow( &temp->outfit_weapon ),
                               OUTFIT_SLOT_WEAPON, cur );
            else if ( xml_isNode( cur, "intrinsic" ) ) {
               const Outfit *o = outfit_get( xml_get( cur ) );
               if ( o == NULL ) {
                  WARN( _( "Ship '%s' has unknown intrinsic outfit '%s'" ),
                        temp->name, xml_get( cur ) );
                  continue;
               }
               if ( temp->outfit_intrinsic == NULL )
                  temp->outfit_intrinsic =
                     (Outfit const **)array_create( Outfit * );
               array_push_back( &temp->outfit_intrinsic, o );
            } else
               WARN( _( "Ship '%s' has unknown slot node '%s'." ), temp->name,
                     cur->name );
         } while ( xml_nextNode( cur ) );
         array_shrink( &temp->outfit_structure );
         array_shrink( &temp->outfit_utility );
         array_shrink( &temp->outfit_weapon );
         continue;
      }

      /* Parse ship stats. */
      if ( xml_isNode( node, "stats" ) ) {
         xmlNodePtr cur = node->children;
         /* Clear if duplicated. */
         if ( temp->stats != NULL ) {
            ss_free( temp->stats );
            temp->stats = NULL;
         }
         do {
            ShipStatList *ll;
            xml_onlyNodes( cur );
            ll = ss_listFromXML( cur );
            if ( ll != NULL ) {
               ll->next    = temp->stats;
               temp->stats = ll;
               continue;
            }
            WARN( _( "Ship '%s' has unknown stat '%s'." ), temp->name,
                  cur->name );
         } while ( xml_nextNode( cur ) );

         /* Load array. */
         ss_sort( &temp->stats );
         ss_statsInit( &temp->stats_array );
         ss_statsMergeFromList( &temp->stats_array, temp->stats );

         /* Create description. */
         if ( temp->stats != NULL ) {
            int i;
            free( temp->desc_stats );
            temp->desc_stats = malloc( STATS_DESC_MAX );
            i = ss_statsListDesc( temp->stats, temp->desc_stats, STATS_DESC_MAX,
                                  0 );
            if ( i <= 0 ) {
               free( temp->desc_stats );
               temp->desc_stats = NULL;
            }
         }

         continue;
      }

      /* Parse tags. */
      if ( xml_isNode( node, "tags" ) ) {
         xmlNodePtr cur = node->children;
         for ( int i = 0; i < array_size( temp->tags ); i++ )
            free( temp->tags[i] );
         array_free( temp->tags );
         temp->tags = array_create( char * );
         do {
            xml_onlyNodes( cur );
            if ( xml_isNode( cur, "tag" ) ) {
               const char *tmp = xml_get( cur );
               if ( tmp != NULL )
                  array_push_back( &temp->tags, strdup( tmp ) );
               continue;
            }
            WARN( _( "Ship '%s' has unknown node in tags '%s'." ), temp->name,
                  cur->name );
         } while ( xml_nextNode( cur ) );
         continue;
      }

      /* Used by on-valid and NSH utils, no in-game meaning. */
      if ( xml_isNode( node, "mission" ) )
         continue;

      DEBUG( _( "Ship '%s' has unknown node '%s'." ), temp->name, node->name );
   } while ( xml_nextNode( node ) );

   /* Post processing. */
   temp->dmg_absorb /= 100.;
   temp->turn *= M_PI / 180.; /* Convert to rad. */

   /* Check license. */
   if ( temp->license && !outfit_licenseExists( temp->license ) )
      WARN( _( "Ship '%s' has inexistent license requirement '%s'!" ),
            temp->name, temp->license );

      /* Ship XML validator */
#define MELEMENT( o, s )                                                       \
   if ( o )                                                                    \
   WARN( _( "Ship '%s' missing '%s' element" ), temp->name, s )
   MELEMENT( temp->name == NULL, "name" );
   MELEMENT( temp->base_type == NULL, "base_type" );
   MELEMENT( ( temp->gfx_path == NULL ), "GFX" );
   MELEMENT( temp->size <= 0., "GFX.size" );
   MELEMENT( temp->class == SHIP_CLASS_NULL, "class" );
   MELEMENT( temp->points == 0, "points" );
   MELEMENT( temp->price == 0, "price" );
   MELEMENT( temp->dt_default <= 0., "time_mod" );
   MELEMENT( temp->fabricator == NULL, "fabricator" );
   MELEMENT( temp->description == NULL, "description" );
   MELEMENT( temp->armour == 0., "armour" );
   MELEMENT( ( temp->cond != NULL ) && ( temp->condstr == NULL ), "condstr" );
   MELEMENT( ( temp->cond == NULL ) && ( temp->condstr != NULL ), "cond" );
   /*MELEMENT(temp->accel==0.,"accel");
   MELEMENT(temp->turn==0.,"turn");
   MELEMENT(temp->speed==0.,"speed");
   MELEMENT(temp->shield==0.,"shield");
   MELEMENT(temp->shield_regen==0.,"shield_regen");
   MELEMENT(temp->energy==0.,"energy");
   MELEMENT(temp->energy_regen==0.,"energy_regen");
   MELEMENT(temp->fuel==0.,"fuel");*/
   MELEMENT( temp->crew == 0, "crew" );
   MELEMENT( temp->mass == 0., "mass" );
   MELEMENT( temp->fuel_consumption == 0, "fuel_consumption" );
   /*MELEMENT(temp->cap_cargo==0,"cargo");
   MELEMENT(temp->cpu==0.,"cpu");*/
#undef MELEMENT

   xmlFreeDoc( doc );

   return 0;
}

/**
 * @brief Renders a 3D ship to a framebuffer.
 */
static void ship_renderFramebuffer3D( const Ship *s, GLuint fbo, double size,
                                      double fw, double fh, double engine_glow,
                                      double t, const glColour *c,
                                      const Lighting *L, const mat4 *H,
                                      int blit, unsigned int flags )
{
   double      scale = ship_aa_scale * size;
   GltfObject *obj   = s->gfx_3d;
   mat4        projection, tex_mat;
   GLint       sbuffer = ceil( scale );

   glBindFramebuffer( GL_FRAMEBUFFER, ship_fbo[0] );

   /* Only clear the necessary area. */
   glEnable( GL_SCISSOR_TEST );
   glScissor( 0, 0, sbuffer, sbuffer );
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   glDisable( GL_SCISSOR_TEST );

   /* Compute projection and texture matrices. */
   projection = mat4_ortho( 0., fw, 0, fh, -1., 1. );
   mat4_scale_xy( &projection, scale * gl_screen.scale,
                  scale * gl_screen.scale );
   tex_mat = mat4_identity();
   mat4_scale_xy( &tex_mat, scale / ship_fbos, scale / ship_fbos );

   /* Actually render. */
   if ( ( engine_glow > 0. ) && ( obj->scene_engine >= 0 ) ) {
      if ( engine_glow >= 1. ) {
         gltf_renderScene( ship_fbo[0], obj, obj->scene_engine, H, t, scale,
                           L );
      } else {
         /* More scissors on the remaining ship fbos. */
         glEnable( GL_SCISSOR_TEST );
         glBindFramebuffer( GL_FRAMEBUFFER, ship_fbo[2] );
         glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
         glBindFramebuffer( GL_FRAMEBUFFER, ship_fbo[1] );
         glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
         glDisable( GL_SCISSOR_TEST );

         /* First render separately. */
         gltf_renderScene( ship_fbo[1], obj, obj->scene_body, H, t, scale, L );
         gltf_renderScene( ship_fbo[2], obj, obj->scene_engine, H, t, scale,
                           L );

         /* Now merge to main framebuffer. */
         glBindFramebuffer( GL_FRAMEBUFFER, ship_fbo[0] );
         gl_renderTextureInterpolateRawH( ship_tex[2], ship_tex[1], engine_glow,
                                          &projection, &tex_mat, &cWhite );

         /* Copy depth over. */
         GLint blitsize = scale;
         glBindFramebuffer( GL_READ_FRAMEBUFFER, ship_fbo[1] );
         glBindFramebuffer( GL_DRAW_FRAMEBUFFER, ship_fbo[0] );
         glBlitFramebuffer( 0, 0, blitsize, blitsize, 0, 0, blitsize, blitsize,
                            GL_DEPTH_BUFFER_BIT, GL_NEAREST );

         /* TODO fix this area, it causes issues with rendering pilots to the
          * framebuffer in the gui. */
      }
   } else
      gltf_renderScene( ship_fbo[0], obj, obj->scene_body, H, t, scale, L );

   /*
    * First do sharpen pass.
    */
   glBindFramebuffer( GL_FRAMEBUFFER, ship_fbo[1] );
   glEnable( GL_SCISSOR_TEST );
   glScissor( 0, 0, sbuffer + ship_aa_scale_base,
              sbuffer + ship_aa_scale_base );
   glClear( GL_COLOR_BUFFER_BIT );
   glDisable( GL_SCISSOR_TEST );

   glUseProgram( shaders.texture_sharpen.program );
   glBindTexture( GL_TEXTURE_2D, ship_tex[0] );

   glEnableVertexAttribArray( shaders.texture_sharpen.vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.texture_sharpen.vertex, 0,
                               2, GL_FLOAT, 0 );

   /* Set shader uniforms. */
   gl_uniformColour( shaders.texture_sharpen.colour, c );
   gl_uniformMat4( shaders.texture_sharpen.projection, &projection );
   gl_uniformMat4( shaders.texture_sharpen.tex_mat, &tex_mat );

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shaders.texture_sharpen.vertex );

   /*
    * Now downsample pass.
    */
   /* Tests show that for 2x AA, linear is visually indifferent from bicubic.
    * The padding is to ensure that there is a one pixel buffer of alpha 0.
    */
   if ( blit ) {
      GLint sin  = sbuffer + (GLint)ship_aa_scale_base;
      GLint sout = ceil( size / gl_screen.scale ) + 1;
      glBindFramebuffer( GL_READ_FRAMEBUFFER, ship_fbo[1] );
      glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fbo );
      if ( flags & OPENGL_TEX_VFLIP ) {
         glBlitFramebuffer( 0, 0, sin, sin, 0, sout, sout, 0,
                            GL_COLOR_BUFFER_BIT, GL_LINEAR );
         /* Depth can only use NEAREST filtering. */
         glBindFramebuffer( GL_READ_FRAMEBUFFER, ship_fbo[0] );
         glBlitFramebuffer( 0, 0, sin, sin, 0, sout, sout, 0,
                            GL_DEPTH_BUFFER_BIT, GL_NEAREST );
      } else {
         glBlitFramebuffer( 0, 0, sin, sin, 0, 0, sout, sout,
                            GL_COLOR_BUFFER_BIT, GL_LINEAR );
         glBindFramebuffer( GL_READ_FRAMEBUFFER, ship_fbo[0] );
         glBlitFramebuffer( 0, 0, sin, sin, 0, 0, sout, sout,
                            GL_DEPTH_BUFFER_BIT, GL_NEAREST );
      }
   } else {
      glBindFramebuffer( GL_FRAMEBUFFER, fbo );
      tex_mat = mat4_identity();
      if ( flags & OPENGL_TEX_VFLIP ) {
         tex_mat.m[1][1] = -1.;
         tex_mat.m[3][1] = 0.5;
      }
      gl_renderTextureRawH( ship_tex[1], &projection, &tex_mat, &cWhite );
   }

   glBindFramebuffer( GL_FRAMEBUFFER, gl_screen.current_fbo );

   /* anything failed? */
   gl_checkErr();
}

/**
 * @brief Renders a ship to a framebuffer.
 */
void ship_renderFramebuffer( const Ship *s, GLuint fbo, double fw, double fh,
                             double dir, double engine_glow, double tilt,
                             double r, int sx, int sy, const glColour *c,
                             const Lighting *L )
{
   if ( c == NULL )
      c = &cWhite;

   if ( s->gfx_3d != NULL ) {
      double t = elapsed_time_mod + r * 300.;

      /* Determine the model transformation. */
      mat4 H = mat4_identity();
      if ( fabs( tilt ) > DOUBLE_TOL ) {
         mat4_rotate( &H, M_PI_2, 0.0, 1.0, 0.0 );
         mat4_rotate( &H, -tilt, 1.0, 0.0, 0.0 );
         mat4_rotate( &H, dir, 0.0, 1.0, 0.0 );
      } else
         mat4_rotate( &H, dir + M_PI_2, 0.0, 1.0, 0.0 );

      ship_renderFramebuffer3D( s, fbo, s->size, fw, fh, engine_glow, t, c, L,
                                &H, 1, 0 );
      /* Already restore current framebuffer. */
   } else {
      double           tx, ty;
      const glTexture *sa, *sb;
      mat4             tmpm;

      glBindFramebuffer( GL_FRAMEBUFFER, fbo );

      sa = s->gfx_space;
      sb = s->gfx_engine;

      /* Only clear the necessary area. */
      glEnable( GL_SCISSOR_TEST );
      glScissor( 0, 0, sa->sw / gl_screen.scale + 1,
                 sa->sh / gl_screen.scale + 1 );
      glClear( GL_COLOR_BUFFER_BIT );
      glDisable( GL_SCISSOR_TEST );

      /* Texture coords */
      tx = sa->sw * (double)( sx ) / sa->w;
      ty = sa->sh * ( sa->sy - (double)sy - 1 ) / sa->h;

      tmpm           = gl_view_matrix;
      gl_view_matrix = mat4_ortho( 0., fw, 0, fh, -1., 1. );

      gl_renderTextureInterpolate( sb, sa, engine_glow, 0., 0., sa->sw, sa->sh,
                                   tx, ty, sa->srw, sa->srh, c );

      gl_view_matrix = tmpm;

      glBindFramebuffer( GL_FRAMEBUFFER, gl_screen.current_fbo );
   }
}

/**
 * @brief Wrapper for threaded loading.
 */
static int ship_parseThread( void *ptr )
{
   ShipThreadData *data = ptr;
   /* Load the ship. */
   data->ret = ship_parse( &data->ship, data->filename, 1 );
   /* Render if necessary. */
   if ( naev_shouldRenderLoadscreen() ) {
      gl_contextSet();
      naev_renderLoadscreen();
      gl_contextUnset();
   }
   return data->ret;
}

/**
 * @brief Loads all the ships in the data files.
 *
 *    @return 0 on success.
 */
int ships_load( void )
{
   char **ship_files;
   int    nfiles;
#if DEBUGGING
   Uint32 time = SDL_GetTicks();
#endif /* DEBUGGING */
   ThreadQueue    *tq       = vpool_create();
   ShipThreadData *shipdata = array_create( ShipThreadData );

   /* Validity. */
   ss_check();

   ship_files = ndata_listRecursive( SHIP_DATA_PATH );
   nfiles     = array_size( ship_files );

   /* Initialize stack if needed. */
   if ( ship_stack == NULL )
      ship_stack = array_create_size( Ship, nfiles );

   /* First pass to find what ships we have to load. */
   for ( int i = 0; i < nfiles; i++ ) {
      if ( ndata_matchExt( ship_files[i], "xml" ) ) {
         ShipThreadData *td = &array_grow( &shipdata );
         memset( td, 0, sizeof( ShipThreadData ) );
         td->filename = ship_files[i];
      } else
         free( ship_files[i] );
   }
   array_free( ship_files );

   /* Enqueue the jobs after the data array is done. */
   SDL_GL_MakeCurrent( gl_screen.window, NULL );
   for ( int i = 0; i < array_size( shipdata ); i++ )
      vpool_enqueue( tq, ship_parseThread, &shipdata[i] );
   /* Wait until done processing. */
   vpool_wait( tq );
   vpool_cleanup( tq );
   SDL_GL_MakeCurrent( gl_screen.window, gl_screen.context );

   /* Properly load the data. */
   for ( int i = 0; i < array_size( shipdata ); i++ ) {
      ShipThreadData *td = &shipdata[i];
      if ( !td->ret )
         array_push_back( &ship_stack, td->ship );
      free( td->filename );
   }
   array_free( shipdata );

   /* Sort so we can use ship_get. */
   qsort( ship_stack, array_size( ship_stack ), sizeof( Ship ), ship_cmp );

   /* Now we do the second pass to resolve inheritance. */
   for ( int i = array_size( ship_stack ) - 1; i >= 0; i-- ) {
      Ship *s   = &ship_stack[i];
      int   ret = ship_parse( s, NULL, 0 );
      if ( ret )
         array_erase( &ship_stack, &s[0], &s[1] );
   }

#if DEBUGGING
   /* Check to see if there are name collisions. */
   for ( int i = 1; i < array_size( ship_stack ); i++ )
      if ( strcmp( ship_stack[i - 1].name, ship_stack[i].name ) == 0 )
         WARN( _( "Duplicated ship name '%s' detected!" ), ship_stack[i].name );
#endif /* DEBUGGING */

   /* Shrink stack. */
   array_shrink( &ship_stack );

   /* Second pass to load Lua. */
   for ( int i = 0; i < array_size( ship_stack ); i++ ) {
      Ship *s = &ship_stack[i];
      /* Update max size. */
      max_size = MAX( max_size, s->size );
      if ( s->lua_file == NULL )
         continue;

      nlua_env env;
      size_t   sz;
      char    *dat = ndata_read( s->lua_file, &sz );
      if ( dat == NULL ) {
         WARN( _( "Ship '%s' failed to read Lua '%s'!" ), s->name,
               s->lua_file );
         continue;
      }

      env        = nlua_newEnv( s->lua_file );
      s->lua_env = env;
      /* TODO limit libraries here. */
      nlua_loadStandard( env );
      nlua_loadGFX( env );
      nlua_loadCamera( env );

      /* Run code. */
      if ( nlua_dobufenv( env, dat, sz, s->lua_file ) != 0 ) {
         WARN( _( "Ship '%s' Lua error:\n%s" ), s->name,
               lua_tostring( naevL, -1 ) );
         lua_pop( naevL, 1 );
         nlua_freeEnv( s->lua_env );
         free( dat );
         s->lua_env = LUA_NOREF;
         continue;
      }
      free( dat );

      /* Check functions as necessary. */
      nlua_getenv( naevL, env, "update_dt" );
      if ( !lua_isnoneornil( naevL, -1 ) )
         s->lua_dt = luaL_checknumber( naevL, -1 );
      lua_pop( naevL, 1 );
      s->lua_descextra = nlua_refenvtype( env, "descextra", LUA_TFUNCTION );
      s->lua_init      = nlua_refenvtype( env, "init", LUA_TFUNCTION );
      s->lua_cleanup   = nlua_refenvtype( env, "cleanup", LUA_TFUNCTION );
      s->lua_update    = nlua_refenvtype( env, "update", LUA_TFUNCTION );
      s->lua_explode_init =
         nlua_refenvtype( env, "explode_init", LUA_TFUNCTION );
      s->lua_explode_update =
         nlua_refenvtype( env, "explode_update", LUA_TFUNCTION );

      /* We're just going to run the script and paste here for now. */
      if ( s->lua_descextra != LUA_NOREF ) {
         free( s->desc_extra );
         s->desc_extra = NULL;
         lua_rawgeti( naevL, LUA_REGISTRYINDEX, s->lua_descextra ); /* f */
         lua_pushnil( naevL );                                      /* f, p */
         lua_pushship( naevL, s );               /* f, p, s */
         if ( nlua_pcall( s->lua_env, 2, 1 ) ) { /* */
            lua_pop( naevL, 1 );
         } else if ( lua_isnoneornil( naevL, -1 ) ) {
            /* Case no return we just pass nothing. */
            lua_pop( naevL, 1 );
         } else {
            s->desc_extra = strdup( luaL_checkstring( naevL, -1 ) );
            lua_pop( naevL, 1 );
         }
      }
   }

   /* Debugging timings. */
   if ( conf.devmode ) {
      DEBUG( n_( "Loaded %d Ship in %.3f s", "Loaded %d Ships in %.3f s",
                 array_size( ship_stack ) ),
             array_size( ship_stack ), ( SDL_GetTicks() - time ) / 1000. );
   } else
      DEBUG(
         n_( "Loaded %d Ship", "Loaded %d Ships", array_size( ship_stack ) ),
         array_size( ship_stack ) );

   ships_resize();
   return 0;
}

void ships_resize( void )
{
   if ( ship_aa_scale > 0. ) {
      for ( int i = 0; i < SHIP_FBO; i++ ) {
         glDeleteFramebuffers( 1, &ship_fbo[i] );
         glDeleteTextures( 1, &ship_tex[i] );
         glDeleteTextures( 1, &ship_texd[i] );
      }
   }

   /* Set up OpenGL rendering stuff. */
   ship_aa_scale = ship_aa_scale_base / gl_screen.scale;
   ship_fbos     = ceil( ship_aa_scale * max_size );
   for ( int i = 0; i < SHIP_FBO; i++ ) {
      gl_fboCreate( &ship_fbo[i], &ship_tex[i], ship_fbos, ship_fbos );
      gl_fboAddDepth( ship_fbo[i], &ship_texd[i], ship_fbos, ship_fbos );
   }
   gl_checkErr();
}

/**
 * @brief Frees all the ships.
 */
void ships_free( void )
{
   /* Clean up opengl. */
   for ( int i = 0; i < SHIP_FBO; i++ ) {
      glDeleteFramebuffers( 1, &ship_fbo[i] );
      glDeleteTextures( 1, &ship_tex[i] );
      glDeleteTextures( 1, &ship_texd[i] );
   }

   /* Now ships. */
   for ( int i = 0; i < array_size( ship_stack ); i++ ) {
      Ship *s = &ship_stack[i];

      /* Free stored strings. */
      free( s->inherits );
      free( s->name );
      free( s->class_display );
      free( s->description );
      free( s->desc_extra );
      free( s->base_type );
      free( s->base_path );
      free( s->fabricator );
      free( s->license );
      free( s->cond );
      free( s->condstr );
      free( s->desc_stats );

      /* Free outfits. */
      for ( int j = 0; j < array_size( s->outfit_structure ); j++ )
         ship_freeSlot( &s->outfit_structure[j] );
      for ( int j = 0; j < array_size( s->outfit_utility ); j++ )
         ship_freeSlot( &s->outfit_utility[j] );
      for ( int j = 0; j < array_size( s->outfit_weapon ); j++ )
         ship_freeSlot( &s->outfit_weapon[j] );
      array_free( s->outfit_structure );
      array_free( s->outfit_utility );
      array_free( s->outfit_weapon );
      array_free( s->outfit_intrinsic );

      ss_free( s->stats );

      /* Free graphics. */
      gltf_free( s->gfx_3d );
      gl_freeTexture( s->gfx_space );
      gl_freeTexture( s->gfx_engine );
      gl_freeTexture( s->_gfx_store );
      free( s->gfx_comm );
      for ( int j = 0; j < array_size( s->gfx_overlays ); j++ )
         gl_freeTexture( s->gfx_overlays[j] );
      array_free( s->gfx_overlays );
      free( s->gfx_path );
      free( s->polygon_path );

      /* Free collision polygons. */
      poly_free( &s->polygon );

      /* Free trail emitters. */
      array_free( s->trail_emitters );

      /* Free tags. */
      for ( int j = 0; j < array_size( s->tags ); j++ )
         free( s->tags[j] );
      array_free( s->tags );

      /* Free Lua. */
      nlua_freeEnv( s->lua_env );
      s->lua_env = LUA_NOREF;
      free( s->lua_file );
   }

   array_free( ship_stack );
   ship_stack = NULL;
}

static void ship_freeSlot( ShipOutfitSlot *s )
{
   outfit_freeSlot( &s->slot );
   free( s->name );
}

/**
 * @brief Gets the maximum size of a ship.
 */
double ship_maxSize( void )
{
   return max_size;
}
