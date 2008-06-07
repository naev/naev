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


#define NEBU_DT_MAX           1.

#define NEBULAE_Z             16 /* Z plane */
#define NEBULAE_PATH          "gen/nebu_%02d.png"


/* The nebulae textures */
static GLuint nebu_textures[NEBULAE_Z];
static int nebu_w = 0;
static int nebu_h = 0;
static int nebu_pw, nebu_ph;

/* Information on rendering */
static int cur_nebu[2] = { 0, 1 };
static unsigned int last_render = 0;


/*
 * prototypes
 */
static int nebu_checkCompat( const char* file );
static void saveNebulae( float *map, const uint32_t w, const uint32_t h, const char* file );
static SDL_Surface* loadNebulae( const char* file );
static void nebu_generate (void);


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

   /* Load each, checking for compatibility and padding */
   glGenTextures( NEBULAE_Z, nebu_textures );
   for (i=0; i<NEBULAE_Z; i++) {
      snprintf( nebu_file, PATH_MAX, NEBULAE_PATH, i );

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

      /* Prepare to load into Opengl */
      nebu_sur = gl_prepareSurface( nebu_sur );
      if ((nebu_sur->w != nebu_pw) || (nebu_sur->h != nebu_ph))
         WARN("Nebulae size doesn't match expected! (%dx%d instead of %dx%d)",
               nebu_sur->w, nebu_sur->h, nebu_pw, nebu_ph );

      /* Load the texture */
      glBindTexture( GL_TEXTURE_2D, nebu_textures[i] );
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

   DEBUG("Loaded %d Nebulae Layers", NEBULAE_Z);
}


/*
 * Cleans up the nebu subsystem
 */
void nebu_exit (void)
{
   glDeleteTextures( NEBULAE_Z, nebu_textures );
}


/*
 * Renders the nebulae
 */
void nebu_render (void)
{
   unsigned int t;
   double dt;
   double tw,th;
   GLfloat col[4];
   int temp;

   /* calculate frame to draw */
   t = SDL_GetTicks();
   dt = (t - last_render) / 1000.;
   if (dt > NEBU_DT_MAX) { /* Time to change */
      temp = cur_nebu[0];
      cur_nebu[0] += cur_nebu[0] - cur_nebu[1];
      cur_nebu[1] = temp;
      if (cur_nebu[0]+1 > NEBULAE_Z)
         cur_nebu[0] = NEBULAE_Z - 2;
      else if (cur_nebu[0] < 0)
         cur_nebu[0] = 1;

      last_render = t;
      dt = 0.;
   }

   col[0] = cPurple.r;
   col[1] = cPurple.g;
   col[2] = cPurple.b;
   col[3] = dt / NEBU_DT_MAX;

   tw = (double)nebu_w / (double)nebu_pw;
   th = (double)nebu_h / (double)nebu_ph;

   /* Set up the targets */
   /* Texture 0 */
   glActiveTexture( GL_TEXTURE0 );
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, nebu_textures[cur_nebu[0]]);

   /* Texture 1 */
   glActiveTexture( GL_TEXTURE1 );
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, nebu_textures[cur_nebu[1]]);

   /* Prepare it */
   glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
   glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE );
   glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_INTERPOLATE );
   /* Colour */
   glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, col );

   /* Arguments */
   /* Arg0 */
   glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_CONSTANT );
   glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE1 );
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

   /* Clean up */
   glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
   glDisable(GL_TEXTURE_2D);
   glActiveTexture( GL_TEXTURE0 );
   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glDisable(GL_TEXTURE_2D);

   /* anything failed? */
   gl_checkErr();
}


/*
 * Forces generation of new nebulae
 */
void nebu_forceGenerate (void)
{
   nebu_w = nebu_h = -9;
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

   /* Generate all the nebulae */
   nebu = noise_genNebulaeMap( w, h, NEBULAE_Z, 5. );
   nfile_dirMakeExist( "gen" );

   /* Save each nebulae as an image */
   for (i=0; i<NEBULAE_Z; i++) {
      snprintf( nebu_file, PATH_MAX, NEBULAE_PATH, i );
      saveNebulae( &nebu[ i*w*h ], w, h, nebu_file );
   }

   /* Cleanup */
   free(nebu);
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

   sur = noise_surfaceFromNebulaeMap( map, w, h );

   snprintf(file_path, PATH_MAX, "%s%s", nfile_basePath(), file );
   SDL_SavePNG( sur, file_path );

   SDL_FreeSurface( sur );
}


/*
 * Loads the nebuale from file.
 */
static SDL_Surface* loadNebulae( const char* file )
{
   char file_path[PATH_MAX];
   SDL_Surface* sur;

   snprintf(file_path, PATH_MAX, "%s%s", nfile_basePath(), file );
   sur = IMG_Load( file_path );
   if (sur == NULL) {
      ERR("Unable to load Nebulae image: %s", file);
      return NULL;
   }
   
   return sur;
}


