/*
 * See Licensing and Copyright notice in naev.h
 */


#include "nebulae.h"

#include <errno.h>

#ifdef _POSIX_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#endif /* _POSIX_SOURCE */

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
#define NEBULAE_PATH          "gen/nebu_%02d.nebu"


/* The nebulae textures */
static GLuint nebu_textures[NEBULAE_Z];
static int nebu_w, nebu_h, nebu_pw, nebu_ph;


/*
 * prototypes
 */
static int nebu_checkCompat( const char* file );
static void saveNebulae( float *map, const uint32_t w, const uint32_t h, const char* file );
static unsigned char* loadNebulae( const char* file, int* w, int* h );


void nebu_init (void)
{
   int i;
   char nebu_file[PATH_MAX];
   unsigned char *nebu_padded;
   int w, h;
   unsigned char *nebu_data;

   /* Set expected sizes */
   nebu_w = SCREEN_W;
   nebu_h = SCREEN_H;
   nebu_pw = gl_pot(nebu_w);
   nebu_ph = gl_pot(nebu_h);

   /* Load each, checking for compatibility and padding */
   nebu_padded = malloc( nebu_pw * nebu_ph );
   glGenTextures( NEBULAE_Z, nebu_textures );
   for (i=0; i<NEBULAE_Z; i++) {
      snprintf( nebu_file, PATH_MAX, NEBULAE_PATH, i );

      if (nebu_checkCompat( nebu_file )) { /* Incompatible */
         LOG("No nebulae found, generating (this may take a while).");

         /* So we generate and reload */
         free(nebu_padded);
         nebu_generate( nebu_w, nebu_h );
         nebu_init();
         return;
      }

      /* Load the file */
      nebu_data = loadNebulae( nebu_file, &w, &h );
      memcpy(nebu_padded, nebu_data, w*h);

      /* Load the texture */
      glBindTexture( GL_TEXTURE_2D, nebu_textures[i] );
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA8, nebu_pw, nebu_ph,
            0, GL_ALPHA, GL_UNSIGNED_BYTE, nebu_padded );
      gl_checkErr();

      free(nebu_data); /* No longer need the data */
   }
   free(nebu_padded);
}


/*
 * Cleans up the nebu subsystem
 */
void nebu_exit (void)
{
   glDeleteTextures( NEBULAE_Z, nebu_textures );
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
   int x,y;
   char *buf;
   unsigned char c;
   int cur;
   ssize_t size;
   char file_path[PATH_MAX];

   size = w*h + 16 + 4;
   buf = malloc(size);

   /* write the header */
   memset(buf, '0', 16);
   snprintf(buf, NEBU_FORMAT_HEADER, "NAEV NEBU v" NEBU_VERSION );
   cur = 16;
   memcpy(&buf[cur], &w, 4);
   memcpy(&buf[cur], &h, 4);
   cur += 8;

   /* Ze body */
   for (y=0; y<(int)h; y++)
      for (x=0; x<(int)w; x++) {
         c = (unsigned char) 255.*map[y*w + x];
         memcpy( &buf[cur++], &c, 1 );
      }

   /* write to a file */
   snprintf(file_path, PATH_MAX, "%s%s", nfile_basePath(), file );
#ifdef _POSIX_SOURCE
   int fd;
   fd = open( file_path, O_WRONLY | O_CREAT | O_TRUNC,
         S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
   if (fd < 0) {
      ERR("Unable to open file %s: %s", file, strerror(errno));
      return;
   }
   if (write( fd, buf, size) != size) {
      ERR("Error writing nebulae to %s: %s", file, strerror(errno));
      return;
   }
   close(fd);
#else /* _POSIX_SOURCE */
#error "Needs implementation."
#endif /* _POSIX_SOURCE */
}


/*
 * Loads the nebuale from file.
 */
static unsigned char* loadNebulae( const char* file, int* w, int* h )
{
   unsigned char* buf;
   char header[16];
   uint32_t tw, th;
   ssize_t len;
   char file_path[PATH_MAX];

#ifdef _POSIX_SOURCE
#define READ(b,l) \
len = read(fd, b, l); \
if (len < l) { \
   WARN("Read too few bytes from %s: %s", file, strerror(errno)); \
   return NULL; \
}   
   int fd;
   snprintf(file_path, PATH_MAX, "%s%s", nfile_basePath(), file );
   fd = open( file, O_RDONLY );
   if (fd < 0) {
      ERR("Unable to open file %s: %s", file_path, strerror(errno));
      return NULL;
   }
   READ(header,16);
   READ(&tw,4);
   READ(&th,4);
   buf = malloc(tw*th);
   READ(buf,tw*th);
#else /* _POSIX_SOURCE */
#error "Needs implementation."
#endif /* _POSIX_SOURCE */
   (*w) = (int) tw;
   (*h) = (int) th;
   return buf;
}


