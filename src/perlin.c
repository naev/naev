/*
* libtcod 1.3.1
* Copyright (c) 2007,2008 J.C.Wilk
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of J.C.Wilk may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY J.C.WILK ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL J.C.WILK BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * @file perlin.c
 *
 * @brief Handles creating noise based on perlin noise.
 *
 * Code tries to handle basically 2d/3d cases, without much genericness
 *  because it needs to be pretty fast.  Originally sped up the code from
 *  about 20 seconds to 8 seconds per Nebula image with the manual loop
 *  unrolling.
 *
 * @note Tried to optimize a while back with SSE and the works, but because
 *       of the nature of how it's implemented in non-linear fashion it just
 *       wound up complicating the code without actually making it faster.
 */


/** @cond */
#include <stdlib.h>
/** @endcond */

#include "perlin.h"

#include "rng.h"

#define SIMPLEX_SCALE 0.5f

/**
 * @brief Structure used for generating noise.
 */
struct perlin_data_s {
   int ndim; /**< Dimension of the noise. */
   unsigned char map[256]; /**< Randomized map of indexes into buffer */
};

#define SWAP(a, b, t)      (t) = (a); (a) = (b); (b) = (t) /**< Swaps two values. */
#define FLOOR(a)           ((int)(a) - ((a) < 0 && (a) != (int)(a))) /**< Limits to 0. */

/**
 * @brief Creates a new perlin noise generator.
 */
perlin_data_t* noise_new (void)
{
   perlin_data_t *pdata;
   int i, j;
   unsigned char tmp;

   /* Create the data. */
   pdata = calloc(sizeof(perlin_data_t),1);

   /* Create the buffer and map. */
   for (i=0; i<256; i++)
      pdata->map[i] = (unsigned char)i;

   while (--i) {
      j = RNG(0, 255);
      SWAP(pdata->map[i], pdata->map[j], tmp);
   }

   return pdata;
}

#define NOISE_SIMPLEX_GRADIENT_1D(n,h,x) { float grad; h &= 0xF; grad=1.0f+(h & 7); if ( h & 8 ) grad = -grad; n = grad * x; }

/**
 * @brief Gets 1D simplex noise for a position.
 *
 *    @param pdata Perlin data to generate noise from.
 *    @param f Position of the noise.
 */
float noise_simplex1( perlin_data_t* pdata, float f[1] )
{
   int i0   = (int)FLOOR( f[0]*SIMPLEX_SCALE );
   int i1   = i0+1;
   float x0 = f[0]*SIMPLEX_SCALE - i0;
   float x1 = x0 - 1.0f;
   float t0 = 1.0f - x0*x0;
   float t1 = 1.0f - x1*x1;
   float n0,n1;

   t0    = t0*t0;
   t1    = t1*t1;
   i0    = pdata->map[i0&0xFF];
   NOISE_SIMPLEX_GRADIENT_1D( n0, i0, x0 );
   n0   *= t0*t0;
   i1    = pdata->map[i1&0xFF];
   NOISE_SIMPLEX_GRADIENT_1D( n1, i1, x1 );
   n1   *= t1*t1;

   return 0.25f * (n0+n1);
}

/**
 * @brief Frees some noise data.
 *
 *    @param pdata Noise data to free.
 */
void noise_delete( perlin_data_t* pdata )
{
   free(pdata);
}
