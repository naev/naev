/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NPNG_H
#  define NPNG_H


#include "SDL.h"


typedef struct npng_s npng_t;


npng_t *npng_open( SDL_RWops *rw );
void npng_close( npng_t *npng );
int npng_dim( npng_t *npng, int *w, int *h );


#endif /* NPNG_H */

