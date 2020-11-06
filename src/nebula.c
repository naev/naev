/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nebula.c
 *
 * @brief Handles rendering and generating the nebula.
 */

#include "nebula.h"

#include "naev.h"

#include <errno.h>

#include "log.h"
#include "opengl.h"
#include "nfile.h"
#include "perlin.h"
#include "rng.h"
#include "menu.h"
#include "player.h"
#include "pause.h"
#include "gui.h"
#include "conf.h"
#include "spfx.h"
#include "npng.h"
#include "camera.h"
#include "nstring.h"
#include "ndata.h"


#define NEBULA_Z             16 /**< Z plane */
#define NEBULA_PUFFS         32 /**< Amount of puffs to generate */
#define NEBULA_PATH_BG       "nebu_bg_%dx%d_%02d.png" /**< Nebula path format. */

#define NEBULA_PUFF_BUFFER   300 /**< Nebula buffer */


/* Externs */
extern void loadscreen_render( double done, const char *msg ); /**< from naev.c */


/* The nebula textures */
static glTexture* nebu_textures[NEBULA_Z]; /**< BG Nebula textures. */
static int nebu_w    = 0; /**< BG Nebula width. */
static int nebu_h    = 0; /**< BG Nebula height. */
static int nebu_pw   = 0; /**< BG Padded Nebula width. */
static int nebu_ph   = 0; /**< BG Padded Nebula height. */

/* Information on rendering */
static int cur_nebu[2]           = { 0, 1 }; /**< Nebulae currently rendering. */
static double nebu_timer         = 0.; /**< Timer since last render. */

/* Nebula properties */
static double nebu_view = 0.; /**< How far player can see. */
static double nebu_dt   = 0.; /**< How fast nebula changes. */

/* puff textures */
static glTexture *nebu_pufftexs[NEBULA_PUFFS]; /**< Nebula puffs. */

/* Misc. */
static int nebu_loaded = 0; /**< Whether the nebula has been loaded. */

/* VBOs */
static gl_vbo *nebu_vboOverlay   = NULL; /**< Overlay VBO. */


/**
 * @struct NebulaPuff
 *
 * @brief Represents a nebula puff.
 */
typedef struct NebulaPuff_ {
   double x; /**< X position. */
   double y; /**< Y position */
   double height; /**< height vs player */
   int tex; /**< Texture */
} NebulaPuff;
static NebulaPuff *nebu_puffs = NULL; /**< Stack of puffs. */
static int nebu_npuffs        = 0; /**< Number of puffs. */
static double puff_x          = 0.;
static double puff_y          = 0.;


/*
 * prototypes
 */
static int nebu_init_recursive( int iter );
static int nebu_checkCompat( const char* file );
static int nebu_loadTexture( SDL_Surface *sur, int w, int h, glTexture **tex );
static int nebu_generate (void);
static int saveNebula( float *map, const uint32_t w, const uint32_t h, const char* file );
static SDL_Surface* loadNebula( const char* file );
static SDL_Surface* nebu_surfaceFromNebulaMap( float* map, const int w, const int h );
/* Puffs. */
static void nebu_generatePuffs (void);
static void nebu_renderPuffs( int below_player );
/* Nebula render methods. */
static void nebu_renderMultitexture( const double dt );


/**
 * @brief Initializes the nebula.
 *
 *    @return 0 on success.
 */
int nebu_init (void)
{
   return nebu_init_recursive( 0 );
}


/**
 * @brief Small wrapper that handles recursivity limits.
 *
 *    @param iter Iteration of recursivity.
 *    @return 0 on success.
 */
static int nebu_init_recursive( int iter )
{
   int i;
   char nebu_file[PATH_MAX];
   SDL_Surface* nebu_sur;
   int ret;

   /* Avoid too much recursivity. */
   if (iter > 3) {
      WARN(_("Unable to generate nebula after 3 attempts, something has really gone wrong!"));
      return -1;
   }

   /* Set expected sizes */
   nebu_w  = gl_screen.rw;
   nebu_h  = gl_screen.rh;
   if (gl_needPOT()) {
      nebu_pw = gl_pot(nebu_w);
      nebu_ph = gl_pot(nebu_h);
   }
   else {
      nebu_pw = nebu_w;
      nebu_ph = nebu_h;
   }

   /* Special code to regenerate the nebula */
   if ((nebu_w == -9) && (nebu_h == -9))
      nebu_generate();

   /* Load each, checking for compatibility and padding */
   for (i=0; i<NEBULA_Z; i++) {
      nsnprintf( nebu_file, PATH_MAX, NEBULA_PATH_BG, nebu_w, nebu_h, i );

      /* Check compatibility. */
      if (nebu_checkCompat( nebu_file ))
         goto no_nebula;

      /* Try to load. */
      nebu_sur = loadNebula( nebu_file );
      if (nebu_sur == NULL)
         goto no_nebula;
      if ((nebu_sur->w != nebu_w) || (nebu_sur->h != nebu_h))
         WARN(_("Nebula raw size doesn't match expected! (%dx%d instead of %dx%d)"),
               nebu_sur->w, nebu_sur->h, nebu_w, nebu_h );

      /* Load the texture */
      ret = nebu_loadTexture( nebu_sur, nebu_pw, nebu_ph, &nebu_textures[i] );
      if (ret)
         goto no_nebula;
   }

   /* Generate puffs after the recursivity stuff. */
   nebu_generatePuffs();

   /* Display loaded nebulas. */
   DEBUG(_("Loaded %d Nebula Layers"), NEBULA_Z);

   nebu_vbo_init();
   nebu_loaded = 1;

   return 0;
no_nebula:
   LOG(_("No nebula found, generating (this may take a while)."));

   /* So we generate and reload */
   ret = nebu_generate();
   if (ret != 0) /* An error has happened - break recursivity*/
      return ret;

   return nebu_init_recursive( iter+1 );
}


/**
 * @brief Initializes the nebula VBO.
 */
void nebu_vbo_init (void)
{
}


int nebu_isLoaded (void)
{
   return nebu_loaded;
}


/**
 * @brief Gets the nebula view radius.
 *
 *    @return The nebula view radius.
 */
double nebu_getSightRadius (void)
{
   return nebu_view;
}


/**
 * @brief Loads sur into tex, checks for expected size of w and h.
 *
 *    @param sur Surface to load into texture.
 *    @param w Expected width of surface.
 *    @param h Expected height of surface.
 *    @param tex Already generated texture to load into.
 *    @return 0 on success;
 */
static int nebu_loadTexture( SDL_Surface *sur, int w, int h, glTexture **tex )
{
   *tex = gl_loadImage(sur, 0);

   if ((w!=0) && (h!=0) &&
         (((*tex)->rw != w) || ((*tex)->rh != h))) {
      WARN(_("Nebula size doesn't match expected! (%dx%d instead of %dx%d)"),
            (*tex)->rw, (*tex)->rh, nebu_pw, nebu_ph );
      return -1;
   }

   return 0;
}


/**
 * @brief Cleans up the nebu subsystem.
 */
void nebu_exit (void)
{
   int i;

   /* Free the Nebula BG. */
   for (i=0; i < NEBULA_Z; i++)
      gl_freeTexture( nebu_textures[i] );

   /* Free the puffs. */
   for (i=0; i<NEBULA_PUFFS; i++)
      gl_freeTexture( nebu_pufftexs[i] );

   if (nebu_vboOverlay != NULL) {
      gl_vboDestroy( nebu_vboOverlay );
      nebu_vboOverlay= NULL;
   }
}


/**
 * @brief Renders the nebula.
 *
 *    @param dt Current delta tick.
 */
void nebu_render( const double dt )
{
   /* Different rendering backends. */
   nebu_renderMultitexture(dt);

   /* Now render the puffs, they are generic. */
   nebu_renderPuffs( 1 );
}


/**
 * @brief Renders the nebula using the multitexture approach.
 *
 *    @param dt Current delta tick.
 */
static void nebu_renderMultitexture( const double dt )
{
   int temp;
   double sx, sy, tw, th;

   /* calculate frame to draw */
   nebu_timer -= dt;
   if (nebu_timer < 0.) { /* Time to change. */
      temp         = cur_nebu[0] - cur_nebu[1];
      cur_nebu[1]  = cur_nebu[0];
      cur_nebu[0] += temp;

      if (cur_nebu[0] >= NEBULA_Z)
         cur_nebu[0] = cur_nebu[1] - 1;
      else if (cur_nebu[0] < 0)
         cur_nebu[0] = cur_nebu[1] + 1;

      /* Change timer. */
      nebu_timer += nebu_dt;

      /* Case it hasn't rendered in a while so it doesn't get buggy. */
      if (nebu_timer < 0)
         nebu_timer = nebu_dt;
   }

   tw = (double)nebu_w / (double)nebu_pw;
   th = (double)nebu_h / (double)nebu_ph;

   /* Compensate possible rumble */
   spfx_getShake( &sx, &sy );

   gl_blitTextureInterpolate(
      nebu_textures[cur_nebu[0]],
      nebu_textures[cur_nebu[1]],
      (nebu_dt - nebu_timer) / nebu_dt,
      -sx, -sy, SCREEN_W, SCREEN_H,
      0, 0, tw, th,
      &cBlue);
}

/**
 * @brief Regenerates the overlay.
 */
void nebu_genOverlay (void)
{
   GLfloat vertex[8];

   /* See if need to generate overlay. */
   if (nebu_vboOverlay == NULL) {
      nebu_vboOverlay = gl_vboCreateStatic( sizeof(GLfloat) *
            ((2+4)*18 + 2*28 + 4*7), NULL );
      vertex[0] = -.5;
      vertex[1] = -.5;
      vertex[2] = .5;
      vertex[3] = -.5;
      vertex[4] = -.5;
      vertex[5] = .5;
      vertex[6] = .5;
      vertex[7] = .5;
      nebu_vboOverlay = gl_vboCreateStatic( sizeof(GLfloat) * 8, vertex );
   }
}


/**
 * @brief Renders the nebula overlay (hides what player can't see).
 *
 *    @param dt Current delta tick.
 */
void nebu_renderOverlay( const double dt )
{
   (void) dt;
   double gx, gy;
   double z;
   gl_Matrix4 projection;

   /* Get GUI offsets. */
   gui_getOffset( &gx, &gy );

   /* Get zoom. */
   z = cam_getZoom();

   /*
    * Renders the puffs
    */
   nebu_renderPuffs( 0 );

   /* Prepare the matrix */
   /* TODO
   ox = gx;
   oy = gy;
   spfx_getShake( &sx, &sy );
   ox += sx;
   oy += sy;
   projection = gl_Matrix4_Translate(gl_view_matrix, SCREEN_W/2.+ox, SCREEN_H/2.+oy, 0);
   projection = gl_Matrix4_Scale(projection, z, z, 1);
   */

   projection = gl_Matrix4_Identity();
   projection = gl_Matrix4_Scale(projection, gl_screen.rw, gl_screen.rh, 1);

   glUseProgram(shaders.nebula.program);
   gl_uniformColor(shaders.nebula.color, &cDarkBlue);
   gl_Matrix4_Uniform(shaders.nebula.projection, projection);
   glUniform2f(shaders.nebula.center, gl_screen.rw / 2, gl_screen.rh / 2);
   glUniform1f(shaders.nebula.radius, nebu_view * z * (1 / gl_screen.scale));

   glEnableVertexAttribArray(shaders.nebula.vertex);
   gl_vboActivateAttribOffset( nebu_vboOverlay, shaders.nebula.vertex, 0, 2, GL_FLOAT, 0 );
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   glUseProgram(0);
   glDisableVertexAttribArray(shaders.nebula.vertex);

   /* Reset puff movement. */
   puff_x = 0.;
   puff_y = 0.;

   gl_checkErr();
}


/**
 * @brief Renders the puffs.
 *
 *    @param below_player Render the puffs below player or above player?
 */
static void nebu_renderPuffs( int below_player )
{
   int i;

   /* Main menu shouldn't have puffs */
   if (menu_isOpen(MENU_MAIN)) return;

   for (i=0; i<nebu_npuffs; i++) {

      /* Separate by layers */
      if ((below_player && (nebu_puffs[i].height < 1.)) ||
            (!below_player && (nebu_puffs[i].height > 1.))) {

         /* calculate new position */
         nebu_puffs[i].x += puff_x * nebu_puffs[i].height;
         nebu_puffs[i].y += puff_y * nebu_puffs[i].height;

         /* Check boundaries */
         if (nebu_puffs[i].x > SCREEN_W + NEBULA_PUFF_BUFFER)
            nebu_puffs[i].x -= SCREEN_W + 2*NEBULA_PUFF_BUFFER;
         else if (nebu_puffs[i].y > SCREEN_H + NEBULA_PUFF_BUFFER)
            nebu_puffs[i].y -= SCREEN_H + 2*NEBULA_PUFF_BUFFER;
         else if (nebu_puffs[i].x < -NEBULA_PUFF_BUFFER)
            nebu_puffs[i].x += SCREEN_W + 2*NEBULA_PUFF_BUFFER;
         else if (nebu_puffs[i].y < -NEBULA_PUFF_BUFFER)
            nebu_puffs[i].y += SCREEN_H + 2*NEBULA_PUFF_BUFFER;

         /* Render */
         gl_blitStatic( nebu_pufftexs[nebu_puffs[i].tex],
               nebu_puffs[i].x, nebu_puffs[i].y, &cLightBlue );
      }
   }
}


/**
 * @brief Moves the nebula puffs.
 */
void nebu_movePuffs( double x, double y )
{
   puff_x += x;
   puff_y += y;
}


/**
 * @brief Prepares the nebualae to be rendered.
 *
 *    @param density Density of the nebula (0-1000).
 *    @param volatility Volatility of the nebula (0-1000).
 */
void nebu_prep( double density, double volatility )
{
   (void)volatility;
   int i;

   nebu_view = 1000. - density;  /* At density 1000 you're blind */
   nebu_dt   = 2000. / (density + 100.); /* Faster at higher density */
   nebu_timer = nebu_dt;

   nebu_npuffs = density/4.;
   nebu_puffs = realloc(nebu_puffs, sizeof(NebulaPuff)*nebu_npuffs);
   for (i=0; i<nebu_npuffs; i++) {
      /* Position */
      nebu_puffs[i].x = (double)RNG(-NEBULA_PUFF_BUFFER,
            SCREEN_W + NEBULA_PUFF_BUFFER);
      nebu_puffs[i].y = (double)RNG(-NEBULA_PUFF_BUFFER,
            SCREEN_H + NEBULA_PUFF_BUFFER);

      /* Maybe make size related? */
      nebu_puffs[i].tex = RNG(0,NEBULA_PUFFS-1);
      nebu_puffs[i].height = RNGF() + 0.2;
   }

   /* Generate the overlay. */
   nebu_genOverlay();
}


/**
 * @brief Forces generation of new nebula on init.
 */
void nebu_forceGenerate (void)
{
   nebu_w = nebu_h = -9; /* \o/ magic numbers */
}


/**
 * @brief Generates the nebula.
 *
 *    @return 0 on success.
 */
static int nebu_generate (void)
{
   int i;
   float *nebu;
   const char *cache;
   char nebu_file[PATH_MAX];
   int ret;

   /* Warn user of what is happening. */
   loadscreen_render( 0.05, _("Generating Nebula (slow, run once)...") );

   /* Try to make the dir first if it fails. */
   cache = nfile_cachePath();
   nfile_dirMakeExist( cache );
   nfile_dirMakeExist( cache, NEBULA_PATH );

   /* Generate all the nebula backgrounds */
   nebu = noise_genNebulaMap( nebu_w, nebu_h, NEBULA_Z, 5. );

   /* Start saving - compression can take a bit. */
   loadscreen_render( 0.05, _("Compressing Nebula layers...") );

   /* Save each nebula as an image */
   for (i=0; i<NEBULA_Z; i++) {
      nsnprintf( nebu_file, PATH_MAX, NEBULA_PATH_BG, nebu_w, nebu_h, i );
      ret = saveNebula( &nebu[ i*nebu_w*nebu_h ], nebu_w, nebu_h, nebu_file );
      if (ret != 0)
         break; /* An error has happened */
   }

   /* Cleanup */
   free(nebu);
   return ret;
}


/**
 * @brief Generates nebula puffs.
 */
static void nebu_generatePuffs (void)
{
   int i;
   int w,h;
   SDL_Surface *sur;
   float *nebu;

   /* Warn user of what is happening. */
   loadscreen_render( 0.05, _("Generating Nebula Puffs...") );

   /* Generate the nebula puffs */
   for (i=0; i<NEBULA_PUFFS; i++) {

      /* Generate the nebula */
      w = h = RNG(20,64);
      nebu = noise_genNebulaPuffMap( w, h, 1. );
      sur = nebu_surfaceFromNebulaMap( nebu, w, h );
      free(nebu);

      /* Load the texture */
      nebu_pufftexs[i] =  gl_loadImage( sur, 0 );
   }
}


/**
 * @brief Checks the validity of a nebula.
 *
 *    @param file Path of the nebula to check (relative to base directory).
 *    @return 0 on success.
 */
static int nebu_checkCompat( const char* file )
{
   /* first check to see if file exists */
   if ( nfile_fileExists( nfile_cachePath(), NEBULA_PATH, file ) == 0 )
      return -1;
   return 0;
}


/**
 * @brief Saves a nebula.
 *
 *    @param map Nebula map to save.
 *    @param w Width of nebula map.
 *    @param h Height of nebula map.
 *    @param file Path to save into.
 *    @return 0 on success.
 */
static int saveNebula( float *map, const uint32_t w, const uint32_t h, const char* file )
{
   char file_path[PATH_MAX];
   SDL_Surface* sur;
   int ret;

   /* fix surface */
   sur = nebu_surfaceFromNebulaMap( map, w, h );

   /* save */
   nsnprintf(file_path, PATH_MAX, "%s"NEBULA_PATH"%s", nfile_cachePath(), file );
   ret = SDL_SavePNG( sur, file_path );

   /* cleanup */
   SDL_FreeSurface( sur );

   return ret;
}


/**
 * @brief Loads the nebulae from file.
 *
 *    @param file Path of the nebula to load.  Relative to base directory.
 *    @return A SDL surface with the nebula.
 */
static SDL_Surface* loadNebula( const char* file )
{
   char file_path[PATH_MAX];
   SDL_Surface* sur;
   SDL_RWops *rw;
   npng_t *npng;

   /* loads the file */
   nsnprintf(file_path, PATH_MAX, "%s"NEBULA_PATH"%s", nfile_cachePath(), file );
   rw    = SDL_RWFromFile( file_path, "rb" );;
   if (rw == NULL) {
      WARN(_("Unable to create rwops from Nebula image: %s"), file);
      return NULL;
   }
   npng  = npng_open( rw );
   if (npng == NULL) {
      WARN(_("Unable to open Nebula image: %s"), file);
      SDL_RWclose( rw );
      return NULL;
   }
   sur   = npng_readSurface( npng, 0, 1 );
   npng_close( npng );
   SDL_RWclose( rw );
   if (sur == NULL) {
      WARN(_("Unable to load Nebula image: %s"), file);
      return NULL;
   }

   return sur;
}



/**
 * @brief Generates a SDL_Surface from a 2d nebula map
 *
 *    @param map Nebula map to use.
 *    @param w Map width.
 *    @param h Map height.
 *    @return A SDL Surface with the nebula.
 */
static SDL_Surface* nebu_surfaceFromNebulaMap( float* map, const int w, const int h )
{
   int i;
   SDL_Surface *sur;
   uint32_t *pix;
   double c;

   /* the good surface */
   sur = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, 32, RGBAMASK );
   pix = sur->pixels;

   /* convert from mapping to actual colours */
   SDL_LockSurface( sur );
   for (i=0; i<h*w; i++) {
      c = map[i];
      pix[i] = RMASK + BMASK + GMASK + (AMASK & (uint32_t)((double)AMASK*c));
   }
   SDL_UnlockSurface( sur );

   return sur;
}
