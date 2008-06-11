/*
 * See Licensing and Copyright notice in naev.h
 */


#include "nebulae.h"

#include <errno.h>

#include "SDL_image.h"

#include "naev.h"
#include "log.h"
#include "opengl.h"
#include "nfile.h"
#include "perlin.h"
#include "rng.h"
#include "menu.h"
#include "player.h"
#include "pause.h"


#define NEBULAE_Z             16 /* Z plane */
#define NEBULAE_PUFFS         32 /* Amount of puffs to generate */
#define NEBULAE_PATH_BG       "gen/nebu_bg_%dx%d_%02d.png"

#define NEBULAE_PUFF_BUFFER   300 /* Nebulae buffer */


/* Externs */
extern double gui_xoff, gui_yoff;
extern Vector2d shake_pos;


/* The nebulae textures */
static GLuint nebu_textures[NEBULAE_Z];
static int nebu_w = 0;
static int nebu_h = 0;
static int nebu_pw, nebu_ph;

/* Information on rendering */
static int cur_nebu[2] = { 0, 1 };
static unsigned int last_render = 0;

/* Nebulae properties */
static double nebu_view = 0.;
static double nebu_dt = 0.;

/* puff textures */
static glTexture *nebu_pufftexs[NEBULAE_PUFFS];

/* puff handling */
typedef struct NebulaePuff_ {
   double x, y; /* Position */
   double a, va; /* alpha, alpha velocity */
   double height; /* height vs player */
   int tex; /* Texture */
} NebulaePuff;
static NebulaePuff *nebu_puffs = NULL;
static int nebu_npuffs = 0;


/*
 * prototypes
 */
static int nebu_checkCompat( const char* file );
static void nebu_loadTexture( SDL_Surface *sur, int w, int h, GLuint tex );
static void nebu_generate (void);
static void nebu_generatePuffs (void);
static void saveNebulae( float *map, const uint32_t w, const uint32_t h, const char* file );
static SDL_Surface* loadNebulae( const char* file );
static SDL_Surface* nebu_surfaceFromNebulaeMap( float* map, const int w, const int h );


/*
 * Initializes the nebulae.
 */
void nebu_init (void)
{
   int i;
   char nebu_file[PATH_MAX];
   SDL_Surface* nebu_sur;

   /* Special code to regenerate the nebulae */
   if ((nebu_w == -9) && (nebu_h == -9))
      nebu_generate();

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
         nebu_generate();
         nebu_init();
         return;
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
}


/*
 * Loads sur into tex, checks for expected size of w and h
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


/*
 * Cleans up the nebu subsystem
 */
void nebu_exit (void)
{
   int i;

   glDeleteTextures( NEBULAE_Z, nebu_textures );

   for (i=0; i<NEBULAE_PUFFS; i++)
      gl_freeTexture( nebu_pufftexs[i] );
}


/*
 * Renders the nebulae
 */
void nebu_render( const double dt )
{
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
   glActiveTexture( GL_TEXTURE0 );
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, nebu_textures[cur_nebu[1]]);

   glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

   /* Texture 1 */
   glActiveTexture( GL_TEXTURE1 );
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
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
      glTranslated(shake_pos.x, shake_pos.y, 0.);

   /* Now render! */
   glBegin(GL_QUADS);
      glMultiTexCoord2d( GL_TEXTURE0, 0., 0. );
      glMultiTexCoord2d( GL_TEXTURE1, 0., 0. );
      glVertex2d( -SCREEN_W/2., -SCREEN_H/2. );

      glMultiTexCoord2d( GL_TEXTURE0, tw, 0. );
      glMultiTexCoord2d( GL_TEXTURE1, tw, 0. );
      glVertex2d(  SCREEN_W/2., -SCREEN_H/2. );

      glMultiTexCoord2d( GL_TEXTURE0, tw, th );
      glMultiTexCoord2d( GL_TEXTURE1, tw, th );
      glVertex2d(  SCREEN_W/2.,  SCREEN_H/2. );
      
      glMultiTexCoord2d( GL_TEXTURE0, 0., th );
      glMultiTexCoord2d( GL_TEXTURE1, 0., th );
      glVertex2d( -SCREEN_W/2.,  SCREEN_H/2. );
   glEnd(); /* GL_QUADS */

   glPopMatrix(); /* GL_PROJECTION */

   /* Set values to defaults */
   glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
   glDisable(GL_TEXTURE_2D);
   glActiveTexture( GL_TEXTURE0 );
   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glDisable(GL_TEXTURE_2D);

   /* Anything failed? */
   gl_checkErr();

   /* Now render the puffs */
   nebu_renderPuffs( dt, 1 );
}


void nebu_renderOverlay( const double dt )
{
#define ANG45     0.70710678118654757
#define COS225    0.92387953251128674
#define SIN225    0.38268343236508978
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
      glTranslated(gui_xoff+shake_pos.x, gui_yoff+shake_pos.y, 0.);


   /*
    * Renders the puffs
    */
   nebu_renderPuffs( dt, 0 );


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
      glVertex2d( -SCREEN_W/2.-gui_xoff, SCREEN_H/2.-gui_yoff );
      glVertex2d( -nebu_view, 0. );
      glVertex2d( -nebu_view*COS225, nebu_view*SIN225 );
      glVertex2d( -nebu_view*ANG45, nebu_view*ANG45 );
      glVertex2d( -nebu_view*SIN225, nebu_view*COS225 );
      glVertex2d( 0., nebu_view );
      glVertex2d( SCREEN_W/2.-gui_xoff, SCREEN_H/2.-gui_yoff );
   glEnd(); /* GL_TRIANGLE_FAN */
   glBegin(GL_TRIANGLE_FAN);
      /* Top Right */
      glVertex2d( SCREEN_W/2.-gui_xoff, SCREEN_H/2.-gui_yoff );
      glVertex2d( 0., nebu_view );
      glVertex2d( nebu_view*SIN225, nebu_view*COS225 );
      glVertex2d( nebu_view*ANG45, nebu_view*ANG45 );
      glVertex2d( nebu_view*COS225, nebu_view*SIN225 );
      glVertex2d( nebu_view, 0. );
      glVertex2d( SCREEN_W/2.-gui_xoff, -SCREEN_H/2.-gui_yoff );
   glEnd(); /* GL_TRIANGLE_FAN */
   glBegin(GL_TRIANGLE_FAN);
      /* Bottom Right */
      glVertex2d( SCREEN_W/2.-gui_xoff, -SCREEN_H/2.-gui_yoff );
      glVertex2d( nebu_view, 0. );
      glVertex2d( nebu_view*COS225, -nebu_view*SIN225 );
      glVertex2d( nebu_view*ANG45, -nebu_view*ANG45 );
      glVertex2d( nebu_view*SIN225, -nebu_view*COS225 );
      glVertex2d( 0., -nebu_view);
      glVertex2d( -SCREEN_W/2.-gui_xoff, -SCREEN_H/2.-gui_yoff );
   glEnd(); /* GL_TRIANGLE_FAN */
   glBegin(GL_TRIANGLE_FAN);
      /* Bottom left */
      glVertex2d( -SCREEN_W/2.-gui_xoff, -SCREEN_H/2.-gui_yoff );
      glVertex2d( 0., -nebu_view);
      glVertex2d( -nebu_view*SIN225, -nebu_view*COS225 );
      glVertex2d( -nebu_view*ANG45, -nebu_view*ANG45 );
      glVertex2d( -nebu_view*COS225, -nebu_view*SIN225 );
      glVertex2d( -nebu_view, 0. );
      glVertex2d( -SCREEN_W/2.-gui_xoff, SCREEN_H/2.-gui_yoff );
   glEnd(); /* GL_TRIANGLE_FAN */

   glPopMatrix(); /* GL_PROJECTION */

   gl_checkErr();
#undef ANG45
#undef COS225
#undef SIN225
}


/*
 * Renders the puffs
 */
void nebu_renderPuffs( const double dt, int below_player )
{
   int i;
   glColour cPuff;

   cPuff.r = cPurple.r;
   cPuff.g = cPurple.g;
   cPuff.b = cPurple.b;

   for (i=0; i<nebu_npuffs; i++) {

      if ((below_player && (nebu_puffs[i].height < 1.)) ||
            (!below_player && (nebu_puffs[i].height > 1.))) {

         /* calculate new position */
         if (!paused) {
            nebu_puffs[i].x -= player->solid->vel.x * nebu_puffs[i].height * dt;
            nebu_puffs[i].y -= player->solid->vel.y * nebu_puffs[i].height * dt;
         }

         /* Calculate new alpha */
         /* Nebu_puffs[i].a += nebu_puffs[i].va * dt; */
         cPuff.a = nebu_puffs[i].a;

         /* Check boundries */
         if (nebu_puffs[i].x > SCREEN_W + NEBULAE_PUFF_BUFFER)
            nebu_puffs[i].x = -NEBULAE_PUFF_BUFFER;
         else if (nebu_puffs[i].y > SCREEN_H + NEBULAE_PUFF_BUFFER)
            nebu_puffs[i].y = -NEBULAE_PUFF_BUFFER;
         else if (nebu_puffs[i].x < -NEBULAE_PUFF_BUFFER)
            nebu_puffs[i].x = SCREEN_W + NEBULAE_PUFF_BUFFER;
         else if (nebu_puffs[i].y < -NEBULAE_PUFF_BUFFER)
            nebu_puffs[i].y = SCREEN_H + NEBULAE_PUFF_BUFFER;

         /* Render */
         gl_blitStatic( nebu_pufftexs[nebu_puffs[i].tex],
               nebu_puffs[i].x, nebu_puffs[i].y, &cPuff );
      }
   }
}


/*
 * Prepares the nebualae
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
      nebu_puffs[i].tex = RNG(0,NEBULAE_PUFFS-1);
      nebu_puffs[i].x = (double)RNG(-NEBULAE_PUFF_BUFFER,
            SCREEN_W + NEBULAE_PUFF_BUFFER);
      nebu_puffs[i].y = (double)RNG(-NEBULAE_PUFF_BUFFER,
            SCREEN_H + NEBULAE_PUFF_BUFFER);
      nebu_puffs[i].a = (double)RNG(20,100)/100.;
      nebu_puffs[i].height = RNGF()*2.;
   }
}


/*
 * Forces generation of new nebulae
 */
void nebu_forceGenerate (void)
{
   nebu_w = nebu_h = -9; /* \o/ magic numbers */
}


/*
 * Generates the nebulae
 */
static void nebu_generate (void)
{
   int i;
   float *nebu;
   char nebu_file[PATH_MAX];
   int w,h;

   w = SCREEN_W;
   h = SCREEN_H;

   /* Generate all the nebulae backgrounds */
   nebu = noise_genNebulaeMap( w, h, NEBULAE_Z, 5. );
   nfile_dirMakeExist( "gen" );

   /* Save each nebulae as an image */
   for (i=0; i<NEBULAE_Z; i++) {
      snprintf( nebu_file, PATH_MAX, NEBULAE_PATH_BG, w, h, i );
      saveNebulae( &nebu[ i*w*h ], w, h, nebu_file );
   }

   /* Cleanup */
   free(nebu);
}


/*
 * Generates nebulae puffs
 */
static void nebu_generatePuffs (void)
{
   int i;
   int w,h;
   SDL_Surface *sur;
   float *nebu;

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


/*
 * Checks the validity of a nebulae. 0 on success.
 */
static int nebu_checkCompat( const char* file )
{
   if (nfile_fileExists(file) == 0) /* first check to see if file exists */
      return -1;
   return 0;
}


/*
 * Saves a nebulae.
 */
static void saveNebulae( float *map, const uint32_t w, const uint32_t h, const char* file )
{
   char file_path[PATH_MAX];
   SDL_Surface* sur;

   /* fix surface */
   sur = nebu_surfaceFromNebulaeMap( map, w, h );

   /* save */
   snprintf(file_path, PATH_MAX, "%s%s", nfile_basePath(), file );
   SDL_SavePNG( sur, file_path );

   /* cleanup */
   SDL_FreeSurface( sur );
}


/*
 * Loads the nebuale from file.
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



/*
 * Generates a SDL_Surface from a 2d nebulae map
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
