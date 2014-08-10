/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef NPNG_H
#  define NPNG_H


#include "SDL.h"

#include <png.h>


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
int npng_pitch( npng_t *npng );

/*
 * Loading.
 */
int npng_readInto( npng_t *npng, png_bytep *row_pointers );
png_bytep npng_readImage( npng_t *npng, png_bytep **rows, int *channels, int *pitch );
SDL_Surface *npng_readSurface( npng_t *npng, int pad_pot, int vflip );

/*
 * Metadata.
 */
int npng_metadata( npng_t *npng, char *txt, char **data );


#endif /* NPNG_H */

