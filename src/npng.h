/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NPNG_H
#  define NPNG_H


/** @cond */
#include <png.h>
#include "SDL.h"
/** @endcond */


typedef struct npng_s npng_t;


/*
 * Open/Close
 */
npng_t *npng_open( SDL_RWops *rw );
void npng_close( npng_t *npng );

/*
 * Info.
 */
int npng_dim( npng_t *npng, png_uint_32 *w, png_uint_32 *h );

/*
 * Loading.
 */
SDL_Surface *npng_readSurface( npng_t *npng, int pad_pot, int vflip );

#endif /* NPNG_H */

