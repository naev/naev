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
 * Loading.
 */
SDL_Surface *npng_readSurface( npng_t *npng );

#endif /* NPNG_H */

