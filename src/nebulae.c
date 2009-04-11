/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nebulae.c
 *
 * @brief Handles rendering and generating the nebulae.
 */

#include "nebulae.h"

#include "naev.h"

#include <errno.h>

#include "SDL_image.h"

#include "log.h"
#include "opengl.h"
#include "nfile.h"
#include "perlin.h"
#include "rng.h"
#include "menu.h"
#include "player.h"
#include "pause.h"
#include "gui.h"


#define NEBULAE_Z             16 /**< Z plane */
#define NEBULAE_PUFFS         32 /**< Amount of puffs to generate */
#define NEBULAE_DIR           "gen/" /**< Directory containing the nebulae stuff. */
#define NEBULAE_PATH_BG       NEBULAE_DIR"nebu_bg_%dx%d_%02d.png" /**< Nebulae path format. */

#define NEBULAE_PUFF_BUFFER   300 /**< Nebulae buffer */


/* Externs */
extern Vector2d shake_pos; /**< from spfx.c */
extern void loadscreen_render( double done, const char *msg ); /**< from naev.c */


/* The nebulae textures */
static GLuint nebu_textures[NEBULAE_Z]; /**< BG Nebulae textures. */
static int nebu_w    = 0; /**< BG Nebulae width. */
static int nebu_h    = 0; /**< BG Nebulae height. */
static int nebu_pw   = 0; /**< BG Padded Nebulae width. */
static int nebu_ph   = 0; /**< BG Padded Nebulae height. */

/* Information on rendering */
static int cur_nebu[2]           = { 0, 1 }; /**< Nebulaes currently rendering. */
static unsigned int last_render  = 0; /**< When they were last rendered. */

/* Nebulae properties */
static double nebu_view = 0.; /**< How far player can see. */
static double nebu_dt   = 0.; /**< How fast nebulae changes. */

/* puff textures */
static glTexture *nebu_pufftexs[NEBULAE_PUFFS]; /**< Nebulae puffs. */

/**
 * @struct NebulaePuff
 *
 * @brief Represents a nebulae puff.
 */
typedef struct NebulaePuff_ {
   double x; /**< X position. */
   double y; /**< Y position */
   double height; /**< height vs player */
   int tex; /**< Texture */
} NebulaePuff;
static NebulaePuff *nebu_puffs   = NULL; /**< Stack of puffs. */
static int nebu_npuffs           = 0; /**< Number of puffs. */


/*
 * prototypes
 */
static int nebu_checkCompat( const char* file );
static void nebu_loadTexture( SDL_Surface *sur, int w, int h, GLuint tex );
static int nebu_generate (void);
static void nebu_generatePuffs (void);
static int saveNebulae( float *map, const uint32_t w, const uint32_t h, const char* file );
static SDL_Surface* loadNebulae( const char* file );
static SDL_Surface* nebu_surfaceFromNebulaeMap( float* map, const int w, const int h );
/* Nebulae render methods. */
static void nebu_renderMultitexture( const double dt );


/**
 * @brief Initializes the nebulae.
 *
 *    @return 0 on success.
 */
int nebu_init (void)
{
   int i;
   char nebu_file[PATH_MAX];
   SDL_Surface* nebu_sur;
   int ret;

   /* Special code to regenerate the nebulae */
   if ((nebu_w == -9) && (nebu_h == -9)) {
      nebu_generate();
   }

   /* Set expected sizes */
   nebu_w = SCREEN_W;
   nebu_h = SCREEN_H;
   nebu_pw = gl_pot(nebu_w);
   nebu_ph = gl_pot(nebu_h);

   nebu_generatePuffs();

   /* Load each, checking for compatibility and padding */
   glGenTextures( NEBULAE_Z, nebu_textures );
   for (i=0; i<NEBULAE_Z; i++) {
      snprintf( nebu_file, PATH_MAX, NEBULAE_PATH_BG, nebu_w, nebu_h, i );

      if (nebu_checkCompat( nebu_file )) { /* Incompatible */
         LOG("No nebulae found, generating (this may take a while).");

         /* So we generate and reload */
         ret = nebu_generate();
         if (ret != 0) /* An error has happened - break recursivity*/
            return ret;

         return nebu_init();
      }

      /* Load the file */
      nebu_sur = loadNebulae( nebu_file );
      if ((nebu_sur->w != nebu_w) || (nebu_sur->h != nebu_h))
         WARN("Nebulae raw size doesn't match expected! (%dx%d instead of %dx%d)",
               nebu_sur->w, nebu_sur->h, nebu_pw, nebu_ph );

      /* Load the texture */
      nebu_loadTexture( nebu_sur, nebu_pw, nebu_ph, nebu_textures[i] );
   }

   DEBUG("Loaded %d Nebulae Layers", NEBULAE_Z);
   return 0;
}


/**
 * @brief Loads sur into tex, checks for expected size of w and h.
 *
 *    @param sur Surface to load into texture.
 *    @param w Expected width of surface.
 *    @param h Expected height of surface.
 *    @param tex Already generated texture to load into.
 */
static void nebu_loadTexture( SDL_Surface *sur, int w, int h, GLuint tex )
{
   SDL_Surface *nebu_sur;

   nebu_sur = gl_prepareSurface( sur );
   if ((w!=0) && (h!=0) &&
         ((nebu_sur->w != w) || (nebu_sur->h != h))) {
      WARN("Nebulae size doesn't match expected! (%dx%d instead of %dx%d)",
            nebu_sur->w, nebu_sur->h, nebu_pw, nebu_ph );
      return;
   }

   /* Load the texture */
   glBindTexture( GL_TEXTURE_2D, tex );
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

   /* Store into opengl saving only alpha channel in video memory */
   SDL_LockSurface( nebu_sur );
   glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, nebu_sur->w, nebu_sur->h,
         0, GL_RGBA, GL_UNSIGNED_BYTE, nebu_sur->pixels );
   SDL_UnlockSurface( nebu_sur );

   SDL_FreeSurface(nebu_sur);
   gl_checkErr();
}


/**
 * @brief Cleans up the nebu subsystem.
 */
void nebu_exit (void)
{
   int i;

   glDeleteTextures( NEBULAE_Z, nebu_textures );

   for (i=0; i<NEBULAE_PUFFS; i++)
      gl_freeTexture( nebu_pufftexs[i] );
}


/**
 * @brief Renders the nebulae.
 *
 *    @param dt Current delta tick.
 */
void nebu_render( const double dt )
{
   if (nglActiveTexture != NULL) {
      nebu_renderMultitexture(dt);
   }

   /* Now render the puffs, they are generic. */
   nebu_renderPuffs( dt, 1 );
}


/**
 * @brief Renders the nebulae using the multitexture approach.
 *
 *    @param dt Current delta tick.
 */
static void nebu_renderMultitexture( const double dt )
{
   (void) dt;
   unsigned int t;
   double ndt;
   double tw,th;
   GLfloat col[4];
   int temp;

   /* calculate frame to draw */
   t = SDL_GetTicks();
   ndt = (t - last_render) / 1000.;
   if (ndt > nebu_dt) { /* Time to change */
      temp = cur_nebu[0];
      cur_nebu[0] += cur_nebu[0] - cur_nebu[1];
      cur_nebu[1] = temp;
      if (cur_nebu[0]+1 > NEBULAE_Z)
         cur_nebu[0] = NEBULAE_Z - 2;
      else if (cur_nebu[0] < 0)
         cur_nebu[0] = 1;

      last_render = t;
      ndt = 0.;
   }

   /* Set the colour */
   col[0] = cPurple.r;
   col[1] = cPurple.g;
   col[2] = cPurple.b;
   col[3] = ndt / nebu_dt;

   tw = (double)nebu_w / (double)nebu_pw;
   th = (double)nebu_h / (double)nebu_ph;

   /* Set up the targets */
   /* Texture 0 */
   nglActiveTexture( GL_TEXTURE0 );
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, nebu_textures[cur_nebu[1]]);

   glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

   /* Texture 1 */
   nglActiveTexture( GL_TEXTURE1 );
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, nebu_textures[cur_nebu[0]]);

   /* Prepare it */
   glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
   glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE );
   glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_INTERPOLATE );
   /* Colour */
   glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, col );

   /* Arguments */
   /* Arg0 */
   glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_CONSTANT );
   glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA );
   /* Arg1 */
   glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT );
   glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PREVIOUS );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA );
   /* Arg2 */
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT );
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_CONSTANT );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA );

   /* Compensate possible rumble */
   if (!paused) {
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
         glTranslated(shake_pos.x, shake_pos.y, 0.);
   }

   /* Now render! */
   glBegin(GL_QUADS);
      nglMultiTexCoord2d( GL_TEXTURE0, 0., 0. );
      nglMultiTexCoord2d( GL_TEXTURE1, 0., 0. );
      glVertex2d( -SCREEN_W/2., -SCREEN_H/2. );

      nglMultiTexCoord2d( GL_TEXTURE0, tw, 0. );
      nglMultiTexCoord2d( GL_TEXTURE1, tw, 0. );
      glVertex2d(  SCREEN_W/2., -SCREEN_H/2. );

      nglMultiTexCoord2d( GL_TEXTURE0, tw, th );
      nglMultiTexCoord2d( GL_TEXTURE1, tw, th );
      glVertex2d(  SCREEN_W/2.,  SCREEN_H/2. );
      
      nglMultiTexCoord2d( GL_TEXTURE0, 0., th );
      nglMultiTexCoord2d( GL_TEXTURE1, 0., th );
      glVertex2d( -SCREEN_W/2.,  SCREEN_H/2. );
   glEnd(); /* GL_QUADS */

   if (!paused)
      glPopMatrix(); /* GL_PROJECTION */

   /* Set values to defaults */
   glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
   glDisable(GL_TEXTURE_2D);
   nglActiveTexture( GL_TEXTURE0 );
   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glDisable(GL_TEXTURE_2D);

   /* Anything failed? */
   gl_checkErr();
}


#define ANG45     0.70710678118654757 /**< 1./sqrt(2) */
#define COS225    0.92387953251128674 /**< cos(225) */
#define SIN225    0.38268343236508978 /**< sin(225) */
/**
 * @brief Renders the nebulae overlay (hides what player can't see).
 *
 *    @param dt Current delta tick.
 */
void nebu_renderOverlay( const double dt )
{
   double gx, gy;

   /* Get GUI offsets. */
   gui_getOffset( &gx, &gy );

   /*
    * Renders the puffs
    */
   nebu_renderPuffs( dt, 0 );

   /* Prepare the matrix */
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
      glTranslated(gx, gy, 0.);
   if (!paused)
      glTranslated(shake_pos.x, shake_pos.y, 0.);

   /*
    * Mask for area player can still see (partially)
    */
   glShadeModel(GL_SMOOTH);
   glBegin(GL_TRIANGLE_FAN);
      ACOLOUR(cPurple, 0.);
      glVertex2d( 0., 0. );
      ACOLOUR(cPurple, 1.);
      glVertex2d( -nebu_view, 0. );
      glVertex2d( -nebu_view*COS225, nebu_view*SIN225 );
      glVertex2d( -nebu_view*ANG45, nebu_view*ANG45 );
      glVertex2d( -nebu_view*SIN225, nebu_view*COS225 );
      glVertex2d( 0., nebu_view );
      glVertex2d( nebu_view*SIN225, nebu_view*COS225 );
      glVertex2d( nebu_view*ANG45, nebu_view*ANG45 );
      glVertex2d( nebu_view*COS225, nebu_view*SIN225 );
      glVertex2d( nebu_view, 0. );
      glVertex2d( nebu_view*COS225, -nebu_view*SIN225 );
      glVertex2d( nebu_view*ANG45, -nebu_view*ANG45 );
      glVertex2d( nebu_view*SIN225, -nebu_view*COS225 );
      glVertex2d( 0., -nebu_view);
      glVertex2d( -nebu_view*SIN225, -nebu_view*COS225 );
      glVertex2d( -nebu_view*ANG45, -nebu_view*ANG45 );
      glVertex2d( -nebu_view*COS225, -nebu_view*SIN225 );
      glVertex2d( -nebu_view, 0. );
   glEnd(); /* GL_TRIANGLE_FAN */


   /*
    * Solid nebulae for areas the player can't see
    */
   glShadeModel(GL_FLAT);
   ACOLOUR(cPurple, 1.);
   glBegin(GL_TRIANGLE_FAN);
      /* Top Left */
      glVertex2d( -SCREEN_W/2.-gx, SCREEN_H/2.-gy );
      glVertex2d( -nebu_view, 0. );
      glVertex2d( -nebu_view*COS225, nebu_view*SIN225 );
      glVertex2d( -nebu_view*ANG45, nebu_view*ANG45 );
      glVertex2d( -nebu_view*SIN225, nebu_view*COS225 );
      glVertex2d( 0., nebu_view );
      glVertex2d( SCREEN_W/2.-gx, SCREEN_H/2.-gy );
   glEnd(); /* GL_TRIANGLE_FAN */
   glBegin(GL_TRIANGLE_FAN);
      /* Top Right */
      glVertex2d( SCREEN_W/2.-gx, SCREEN_H/2.-gy );
      glVertex2d( 0., nebu_view );
      glVertex2d( nebu_view*SIN225, nebu_view*COS225 );
      glVertex2d( nebu_view*ANG45, nebu_view*ANG45 );
      glVertex2d( nebu_view*COS225, nebu_view*SIN225 );
      glVertex2d( nebu_view, 0. );
      glVertex2d( SCREEN_W/2.-gx, -SCREEN_H/2.-gy );
   glEnd(); /* GL_TRIANGLE_FAN */
   glBegin(GL_TRIANGLE_FAN);
      /* Bottom Right */
      glVertex2d( SCREEN_W/2.-gx, -SCREEN_H/2.-gy );
      glVertex2d( nebu_view, 0. );
      glVertex2d( nebu_view*COS225, -nebu_view*SIN225 );
      glVertex2d( nebu_view*ANG45, -nebu_view*ANG45 );
      glVertex2d( nebu_view*SIN225, -nebu_view*COS225 );
      glVertex2d( 0., -nebu_view);
      glVertex2d( -SCREEN_W/2.-gx, -SCREEN_H/2.-gy );
   glEnd(); /* GL_TRIANGLE_FAN */
   glBegin(GL_TRIANGLE_FAN);
      /* Bottom left */
      glVertex2d( -SCREEN_W/2.-gx, -SCREEN_H/2.-gy );
      glVertex2d( 0., -nebu_view);
      glVertex2d( -nebu_view*SIN225, -nebu_view*COS225 );
      glVertex2d( -nebu_view*ANG45, -nebu_view*ANG45 );
      glVertex2d( -nebu_view*COS225, -nebu_view*SIN225 );
      glVertex2d( -nebu_view, 0. );
      glVertex2d( -SCREEN_W/2.-gx, SCREEN_H/2.-gy );
   glEnd(); /* GL_TRIANGLE_FAN */

   glPopMatrix(); /* GL_PROJECTION */

   gl_checkErr();
}
#undef ANG45
#undef COS225
#undef SIN225


/**
 * @brief Renders the puffs.
 *
 *    @param dt Current delta tick.
 *    @param below_player Render the puffs below player or above player?
 */
void nebu_renderPuffs( const double dt, int below_player )
{
   int i;

   /* Main menu shouldn't have puffs */
   if (menu_isOpen(MENU_MAIN)) return;

   for (i=0; i<nebu_npuffs; i++) {

      /* Seperate by layers */
      if ((below_player && (nebu_puffs[i].height < 1.)) ||
            (!below_player && (nebu_puffs[i].height > 1.))) {

         /* calculate new position */
         if (!paused && (player!=NULL)) {
            nebu_puffs[i].x -= player->solid->vel.x * nebu_puffs[i].height * dt;
            nebu_puffs[i].y -= player->solid->vel.y * nebu_puffs[i].height * dt;
         }

         /* Check boundries */
         if (nebu_puffs[i].x > SCREEN_W + NEBULAE_PUFF_BUFFER)
            nebu_puffs[i].x -= SCREEN_W + 2*NEBULAE_PUFF_BUFFER;
         else if (nebu_puffs[i].y > SCREEN_H + NEBULAE_PUFF_BUFFER)
            nebu_puffs[i].y -= SCREEN_H + 2*NEBULAE_PUFF_BUFFER;
         else if (nebu_puffs[i].x < -NEBULAE_PUFF_BUFFER)
            nebu_puffs[i].x += SCREEN_W + 2*NEBULAE_PUFF_BUFFER;
         else if (nebu_puffs[i].y < -NEBULAE_PUFF_BUFFER)
            nebu_puffs[i].y += SCREEN_H + 2*NEBULAE_PUFF_BUFFER;

         /* Render */
         gl_blitStatic( nebu_pufftexs[nebu_puffs[i].tex],
               nebu_puffs[i].x, nebu_puffs[i].y, &cPurple );
      }
   }
}


/**
 * @brief Prepares the nebualae to be rendered.
 *
 *    @param density Density of the nebulae (0-1000).
 *    @param volatility Volatility of the nebulae (0-1000).
 */
void nebu_prep( double density, double volatility )
{
   (void)volatility;
   int i;

   nebu_view = 1000. - density;  /* At density 1000 you're blind */
   nebu_dt = 2000. / (density + 100.); /* Faster at higher density */

   nebu_npuffs = density/4.;
   nebu_puffs = realloc(nebu_puffs, sizeof(NebulaePuff)*nebu_npuffs);
   for (i=0; i<nebu_npuffs; i++) {
      /* Position */
      nebu_puffs[i].x = (double)RNG(-NEBULAE_PUFF_BUFFER,
            SCREEN_W + NEBULAE_PUFF_BUFFER);
      nebu_puffs[i].y = (double)RNG(-NEBULAE_PUFF_BUFFER,
            SCREEN_H + NEBULAE_PUFF_BUFFER);
      
      /* Maybe make size related? */
      nebu_puffs[i].tex = RNG(0,NEBULAE_PUFFS-1);
      nebu_puffs[i].height = RNGF() + 0.2;
   }
}


/**
 * @brief Forces generation of new nebulae on init.
 */
void nebu_forceGenerate (void)
{
   nebu_w = nebu_h = -9; /* \o/ magic numbers */
}


/**
 * @brief Generates the nebulae.
 *
 *    @return 0 on success.
 */
static int nebu_generate (void)
{
   int i;
   float *nebu;
   char nebu_file[PATH_MAX];
   int w,h;
   int ret;

   /* Warn user of what is happening. */
   loadscreen_render( 0.05, "Generating Nebulae (slow, run once)..." );

   /* Get resolution to create at. */
   w = SCREEN_W;
   h = SCREEN_H;

   /* Try to make the dir first if it fails. */
   nfile_dirMakeExist( "%s"NEBULAE_DIR, nfile_basePath() );

   /* Generate all the nebulae backgrounds */
   nebu = noise_genNebulaeMap( w, h, NEBULAE_Z, 5. );

   /* Start saving - compression can take a bit. */
   loadscreen_render( 0.05, "Compressing Nebulae layers..." );

   /* Save each nebulae as an image */
   for (i=0; i<NEBULAE_Z; i++) {
      snprintf( nebu_file, PATH_MAX, NEBULAE_PATH_BG, w, h, i );
      ret = saveNebulae( &nebu[ i*w*h ], w, h, nebu_file );
      if (ret != 0) break; /* An error has happenend */
   }

   /* Cleanup */
   free(nebu);
   return ret;
}


/**
 * @brief Generates nebulae puffs.
 */
static void nebu_generatePuffs (void)
{
   int i;
   int w,h;
   SDL_Surface *sur;
   float *nebu;

   /* Warn user of what is happening. */
   loadscreen_render( 0.05, "Generating Nebulae Puffs..." );

   /* Generate the nebulae puffs */
   for (i=0; i<NEBULAE_PUFFS; i++) {

      /* Generate the nebulae */
      w = h = RNG(20,64);
      nebu = noise_genNebulaePuffMap( w, h, 1. );
      sur = nebu_surfaceFromNebulaeMap( nebu, w, h );
      free(nebu);
      
      /* Load the texture */
      nebu_pufftexs[i] =  gl_loadImage( sur );
   }
}


/**
 * @brief Checks the validity of a nebulae.
 *
 *    @param file Path of the nebulae to check (relative to base directory).
 *    @return 0 on success.
 */
static int nebu_checkCompat( const char* file )
{
   /* first check to see if file exists */
   if (nfile_fileExists("%s%s", nfile_basePath(), file) == 0)
      return -1;
   return 0;
}


/**
 * @brief Saves a nebulae.
 *
 *    @param map Nebulae map to save.
 *    @param w Width of nebulae map.
 *    @param h Height of nebulae map.
 *    @param file Path to save into.
 *    @return 0 on success.
 */
static int saveNebulae( float *map, const uint32_t w, const uint32_t h, const char* file )
{
   char file_path[PATH_MAX];
   SDL_Surface* sur;
   int ret;

   /* fix surface */
   sur = nebu_surfaceFromNebulaeMap( map, w, h );

   /* save */
   ret = 0;
   snprintf(file_path, PATH_MAX, "%s%s", nfile_basePath(), file );
   ret = SDL_SavePNG( sur, file_path );

   /* cleanup */
   SDL_FreeSurface( sur );

   return ret;
}


/**
 * @brief Loads the nebuale from file.
 *
 *    @param file Path of the nebulae to load.  Relative to base directory.
 *    @return A SDL surface with the nebulae.
 */
static SDL_Surface* loadNebulae( const char* file )
{
   char file_path[PATH_MAX];
   SDL_Surface* sur;

   /* loads the file */
   snprintf(file_path, PATH_MAX, "%s%s", nfile_basePath(), file );
   sur = IMG_Load( file_path );
   if (sur == NULL) {
      ERR("Unable to load Nebulae image: %s", file);
      return NULL;
   }
   
   return sur;
}



/**
 * @brief Generates a SDL_Surface from a 2d nebulae map
 *
 *    @param map Nebulae map to use.
 *    @param w Map width.
 *    @param h Map height.
 *    @return A SDL Surface with the nebulae.
 */
static SDL_Surface* nebu_surfaceFromNebulaeMap( float* map, const int w, const int h )
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
