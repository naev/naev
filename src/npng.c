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
#include "opengl_tex.h"


/**
 * @brief Wrapper to libpng stuff.
 */
struct npng_s {
   SDL_RWops*  rw; /**< SDL_RWops to read data from. */
   int start; /**< Start position to read from. */
   png_structp png_ptr; /**< PNG struct pointer. */
   png_infop   info_ptr; /**< PNG info struct pointer. */

   /* Text information */
   png_textp text_ptr; /**< Pointer to texts. */
   int num_text; /**< Number of texts. */
};


/*
 * Prototypes.
 */
static void npng_read( png_structp png_ptr, png_bytep data, png_size_t len );
static int npng_info( npng_t *npng );


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
   npng->png_ptr  = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
   if (npng->png_ptr == NULL) {
      WARN("png_create_read_struct failed");
      goto ERR_FAIL;
   }
   npng->info_ptr = png_create_info_struct( npng->png_ptr );
   if (npng->info_ptr == NULL) {
      WARN("png_create_info_struct failed");
      goto ERR_FAIL;
   }

   /* Check header. */
   SDL_RWread( rw, header, 8, 1 );
   if (png_sig_cmp(header, 0, 8)) {
      WARN("RWops not recognized as a PNG file.");
      goto ERR_FAIL;
   }

   /* Set up for reading. */
   png_set_read_fn( npng->png_ptr, (png_voidp) rw, npng_read );

   /* Set up long jump for IO. */
   if (setjmp( png_jmpbuf( npng->png_ptr )) ) {
      WARN("Error during setjmp");
      goto ERR_FAIL;
   }

   /* We've already checked sig. */
   png_set_sig_bytes( npng->png_ptr, 8 );

   /* Get start. */
   npng->start = SDL_RWtell( npng->rw );

   /* Get info. */
   npng_info( npng );

   /* Load text. */
   png_get_text( npng->png_ptr, npng->info_ptr, &npng->text_ptr, &npng->num_text );

   return npng;

ERR_FAIL:
#if 0 /* Memory leaks are better than segfaults. */
   if (npng != NULL) {
      if (npng->png_ptr != NULL)
         png_destroy_read_struct( &npng->png_ptr, (npng->info_ptr != NULL) ? &npng->info_ptr : NULL, NULL );
      free(npng);
   }
#endif
   return NULL;
}


/**
 * @brief Closes a npng_t struct.
 *
 *    @param npng Struct to close.
 */
void npng_close( npng_t *npng )
{
   png_destroy_read_struct( &npng->png_ptr, &npng->info_ptr, NULL );
   free( npng );
}


/**
 * @brief Initializes the npng.
 */
static int npng_info( npng_t *npng )
{
   png_uint_32 width, height;
   int bit_depth, color_type, interlace_type;
   /*double display_exponent, gamma;*/

   /* Read information. */
   png_read_info( npng->png_ptr, npng->info_ptr );

   /* Read header stuff. */
   png_get_IHDR( npng->png_ptr, npng->info_ptr, &width, &height,
         &bit_depth, &color_type, &interlace_type, NULL, NULL );

   /* Set Interlace handling if necessary. */
   if (interlace_type != PNG_INTERLACE_NONE)
      png_set_interlace_handling( npng->png_ptr );

   /* Strip down from 16 bit to 8 bit. */
   png_set_strip_16( npng->png_ptr );

   /* Extract small bits into separate bytes. */
   png_set_packing( npng->png_ptr );

   /* Expand palette to RGB. */
   if (color_type == PNG_COLOR_TYPE_PALETTE)
      png_set_expand( npng->png_ptr );
   /* Expand low bit grayscale to 8 bit. */
   if ((color_type == PNG_COLOR_TYPE_GRAY) && (bit_depth < 8))
      png_set_expand( npng->png_ptr );
   /* Expand tRNS data to alpha channel. */
   if (png_get_valid( npng->png_ptr, npng->info_ptr, PNG_INFO_tRNS) )
      png_set_expand( npng->png_ptr );

   /* Set grayscale to 8 bits. */
   if ((color_type == PNG_COLOR_TYPE_GRAY) ||
         (color_type == PNG_COLOR_TYPE_GRAY_ALPHA))
      png_set_gray_to_rgb( npng->png_ptr );

   /* Set gamma. */
   /*if (png_get_gAMA( npng->png_ptr, info_ptr, &gamma) )
      png_set_gamma( npng->png_ptr, display_exponent, gamma );*/

   /* Fill alpha. */
   png_set_filler( npng->png_ptr, 0xff, PNG_FILLER_AFTER );

   /* Update information. */
   png_read_update_info( npng->png_ptr, npng->info_ptr );

   /* Success. */
   return 0;
}


/**
 * @brief Gets the dimensions of a png.
 *
 *    @param npng PNG to get dimensions of.
 *    @param[out] w Width of the png.
 *    @param[out] h Height of the png.
 */
int npng_dim( npng_t *npng, png_uint_32 *w, png_uint_32 *h )
{
   /* Get data.  */
   *w = png_get_image_width( npng->png_ptr, npng->info_ptr );
   *h = png_get_image_height( npng->png_ptr, npng->info_ptr );

   return 0;
}


/**
 * @brief Gets the pitch of a png.
 */
int npng_pitch( npng_t *npng )
{
   /* Make sure loaded info. */
   return png_get_rowbytes( npng->png_ptr, npng->info_ptr );
}


/**
 * @brief Reads the PNG into a set of rows.
 */
int npng_readInto( npng_t *npng, png_bytep *row_pointers )
{
   png_uint_32 width, height;
   int bit_depth, color_type, interlace_type;

   /* Read information. */
   png_get_IHDR( npng->png_ptr, npng->info_ptr, &width, &height,
         &bit_depth, &color_type, &interlace_type, NULL, NULL );

   /* Go back to position. */
   /*SDL_RWseek( npng->rw, npng->start, RW_SEEK_SET );*/

   /* Read the entire image in one go */
   png_read_image( npng->png_ptr, row_pointers );

   /* In case we want more IDAT stuff. */
   png_read_end( npng->png_ptr, npng->info_ptr );

   /* Success. */
   return 0;
}


/**
 * @brief Reads the png data as 32 bit format.
 */
png_bytep npng_readImage( npng_t *npng, png_bytep **rows, int *channels, int *pitch )
{
   png_bytep image_data;
   png_uint_32 width, height;
   png_bytep *row_pointers;
   png_uint_32  i, rowbytes;

   /* Get info. */
   npng_dim( npng, &width, &height );
   rowbytes = npng_pitch( npng );

   /* Create the array of pointers to image data */
   image_data = malloc( rowbytes * height );
   if (image_data == NULL)
      ERR( "Out of Memory" );
   row_pointers = malloc( sizeof(png_bytep) * height );
   if (row_pointers == NULL)
      ERR( "Out of Memory" );
   for (i=0; i<height; i++)
      row_pointers[i] = image_data + i*rowbytes;

   /* Return data. */
   if (channels != NULL)
      *channels = png_get_channels( npng->png_ptr, npng->info_ptr );
   if (pitch != NULL)
      *pitch = rowbytes;
   if (rows != NULL)
      *rows = row_pointers;
   else
      free( row_pointers );
   return image_data;
}


/**
 * @brief Reads a PNG image into a surface.
 *
 *    @param npng PNG image to load.
 *    @return Surface with data from the PNG image.
 */
SDL_Surface *npng_readSurface( npng_t *npng, int pad_pot, int vflip )
{
   png_bytep *row_pointers;
   png_uint_32 width, height, row, rheight;
   SDL_Surface *surface;
   int channels;
   Uint32 Rmask, Gmask, Bmask, Amask;
   int bit_depth, color_type, interlace_type;

   /* Read information. */
   channels = png_get_channels( npng->png_ptr, npng->info_ptr );
   png_get_IHDR( npng->png_ptr, npng->info_ptr, &width, &height,
         &bit_depth, &color_type, &interlace_type, NULL, NULL );

   /* Pad POT if needed. */
   rheight = height;
   if (pad_pot) {
      width    = gl_pot( width );
      height   = gl_pot( height );
   }

   /* Allocate the SDL surface to hold the image */
   if (SDL_BYTEORDER == SDL_LIL_ENDIAN) {
      Rmask = 0x000000FF;
      Gmask = 0x0000FF00;
      Bmask = 0x00FF0000;
      Amask = (channels == 4) ? 0xFF000000 : 0;
   }
   else {
      int s = (channels == 4) ? 0 : 8;
      Rmask = 0xFF000000 >> s;
      Gmask = 0x00FF0000 >> s;
      Bmask = 0x0000FF00 >> s;
      Amask = 0x000000FF >> s;
   }
   surface = SDL_CreateRGBSurface( SDL_SWSURFACE, width, height,
         bit_depth*channels, Rmask, Gmask, Bmask, Amask );
   if (surface == NULL) {
      ERR( "Out of Memory" );
      return NULL;
   }
   /*if (bit_depth*channels < npng_pitch( npng )) DEBUG(" %d / %d ", bit_depth*channels, npng_pitch( npng ) );*/

   /* Create the array of pointers to image data */
   row_pointers = malloc( sizeof(png_bytep) * rheight );
   if (row_pointers == NULL) {
      ERR( "Out of Memory" );
      return NULL;
   }
   for (row=0; row<rheight; row++) { /* We only need to go to real height, not full height. */
      row_pointers[ vflip ? rheight-row-1 : row ] = (png_bytep)
         (Uint8 *) surface->pixels + row * surface->pitch;
   }

   /* Load the data. */
   npng_readInto( npng, row_pointers );

   /* Free rows. */
   free( row_pointers );
   return surface;
}


/**
 * @brief Gets metadat by name.
 *
 *    @param txt Text name of the metadata to get.
 *    @param[out] Data gotten.
 *    @return The length of the metadata or -1 if not found.
 */
int npng_metadata( npng_t *npng, char *txt, char **data )
{
   int i;

   for (i=0; i<npng->num_text; i++) {
      if (strcmp( txt, npng->text_ptr[i].key )==0) {
         *data = npng->text_ptr[i].text;
         return npng->text_ptr[i].text_length;
      }
   }

   return -1;
}



