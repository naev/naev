/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file npng.c
 *
 * @brief A wrapper around some of the more complex libpng functions.
 */

#include "npng.h"

#include "naev.h"

#include <png.h>

#include "log.h"


/**
 * @brief Wrapper to libpng stuff.
 */
struct npng_s {
   SDL_RWops*  rw; /**< SDL_RWops to read data from. */
   png_structp png_ptr; /**< PNG struct pointer. */
   png_infop   info_ptr; /**< PNG info struct pointer. */
};


/*
 * Prototypes.
 */
static void npng_read( png_structp png_ptr, png_bytep data, png_size_t len );


/**
 * @brief PNG read wrapper using SDL_RWops.
 */
static void npng_read( png_structp png_ptr, png_bytep data, png_size_t len )
{
   SDL_RWops *rw;

   /* Get RWops. */
   rw = (SDL_RWops*) png_get_io_ptr( png_ptr );

   /* Read data. */
   SDL_RWread( rw, data, len, 1 );
}


/**
 * @brief Opens an npng struct from an SDL_RWops. It does not close the RWops.
 *
 *    @param rw SDL_RWops to create npng from.
 *    @return npng created from rw.
 */
npng_t *npng_open( SDL_RWops *rw )
{
   png_byte header[8]; /* Maximum size to check. */
   npng_t *npng;

   /* Allocate memory. */
   npng = malloc( sizeof(npng_t) );
   if (npng == NULL) {
      WARN("Out of memory.");
      return NULL;
   }
   memset( npng, 0, sizeof(npng_t) );

   /* Set up struct. */
   npng->rw       = rw;
   npng->png_ptr  = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (npng->png_ptr == NULL) {
      WARN("png_create_read_struct failed");
      return NULL;
   }
   npng->info_ptr = png_create_info_struct( npng->png_ptr );
   if (npng->info_ptr == NULL) {
      WARN("png_create_info_struct failed");
      return NULL;
   }

   /* Check header. */
   SDL_RWread( rw, header, 8, 1 );
   if (png_sig_cmp(header, 0, 8)) {
      WARN("RWops not recognized as a PNG file.");
      return NULL;
   }

   /* Set up for reading. */
   png_set_read_fn( npng->png_ptr, (voidp) rw, npng_read );

   /* Set up long jump for IO. */
   if (setjmp(png_jmpbuf( npng->png_ptr ))) {
      WARN("Error during init_io");
      return NULL;
   }
   
   /* We've already checked sig. */
   png_set_sig_bytes( npng->png_ptr, 8 );

   /* Read information. */
   png_read_info( npng->png_ptr, npng->info_ptr );

   return npng;
}


/**
 * @brief Closes an npng_t struct.
 *
 *    @param npng Struct to close.
 */
void npng_close( npng_t *npng )
{
   png_destroy_read_struct( &npng->png_ptr, &npng->info_ptr, NULL );
   free( npng );
}


int npng_dim( npng_t *npng, int *w, int *h )
{
   /* Get data.  */
   *w = png_get_image_width( npng->png_ptr, npng->info_ptr );
   *h = png_get_image_height( npng->png_ptr, npng->info_ptr );

   return 0;
}


