/*
 * See Licensing and Copyright notice in naev.h
 */


#include "nebulae.h"

#include <errno.h>

#include "SDL_image.h"

#include "naev.h"
#include "log.h"
#include "nfile.h"
#include "perlin.h"


/*
 * CUSTOM NEBULAE FORMAT
 *
 * Header (16 byte string)
 * Dimensions (4 byte w and 4 byte h)
 * Body (1 byte per pixel)
 */


#define NEBU_FORMAT_HEADER    16 /* Size of header */
#define NEBU_VERSION          "1" /* Will be used for version checking */


#define NEBULAE_Z             32 /* Z plane */
#define NEBULAE_PATH          "gen/nebu_%02d.png"


/* The nebulae textures */
static GLuint nebu_textures[NEBULAE_Z];
static int nebu_w, nebu_h, nebu_pw, nebu_ph;


/*
 * prototypes
 */
static int nebu_checkCompat( const char* file );
static void saveNebulae( float *map, const uint32_t w, const uint32_t h, const char* file );
static SDL_Surface* loadNebulae( const char* file );


/*
 * Initializes the nebulae.
 */
void nebu_init (void)
{
   int i;
   char nebu_file[PATH_MAX];
   SDL_Surface* nebu_sur;

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
         nebu_generate( nebu_w, nebu_h );
         nebu_init();
         return;
      }

      /* Load the file */
      nebu_sur = loadNebulae( nebu_file );
      if ((nebu_sur->w != nebu_w) || (nebu_sur->h != nebu_h))
         WARN("Nebulae raw size doesn't match expected! (%dx%d instead of %dx%d)",
               nebu_sur->w, nebu_sur->h, nebu_pw, nebu_ph );


      nebu_sur = gl_prepareSurface( nebu_sur );
      if ((nebu_sur->w != nebu_pw) || (nebu_sur->h != nebu_ph))
         WARN("Nebulae size doesn't match expected! (%dx%d instead of %dx%d)",
               nebu_sur->w, nebu_sur->h, nebu_pw, nebu_ph );

      /* Load the texture */
      glBindTexture( GL_TEXTURE_2D, nebu_textures[i] );
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

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
   int n;
   double x,y, w,h, tx,ty, tw,th;

   n = 0;

   x = -SCREEN_W/2.;
   y = -SCREEN_H/2.;

   w = SCREEN_W;
   h = SCREEN_W;

   tx = 0.;
   ty = 0.;

   tw = nebu_w / nebu_pw;
   th = nebu_h / nebu_ph;

   glEnable(GL_TEXTURE_2D);
   /*glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);*/
   glBindTexture( GL_TEXTURE_2D, nebu_textures[n]);
   COLOUR(cPurple);
   glBegin(GL_QUADS);
      glTexCoord2d( tx, ty);
      glVertex2d( x, y );

      glTexCoord2d( tx + tw, ty);
      glVertex2d( x + w, y );

      glTexCoord2d( tx + tw, ty + th);
      glVertex2d( x + w, y + h );

      glTexCoord2d( tx, ty + th);
      glVertex2d( x, y + h );
   glEnd(); /* GL_QUADS */
   glDisable(GL_TEXTURE_2D);

   /* anything failed? */
   gl_checkErr();
}


/*
 * Forces generation of new nebulae
 */
void nebu_generate( const int w, const int h )
{
   int i;
   float *nebu;
   char nebu_file[PATH_MAX];

   /* Generate all the nebulae */
   nebu = noise_genNebulaeMap( w, h, NEBULAE_Z, 15. );
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


