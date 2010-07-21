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


#include "perlin.h"

#include "naev.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"

#include "log.h"
#include "rng.h"
#include "nfile.h"


#define TCOD_NOISE_MAX_OCTAVES            4 /**< Default octaves for noise. */
#define TCOD_NOISE_DEFAULT_HURST          0.5 /**< Default hurst for noise. */
#define TCOD_NOISE_DEFAULT_LACUNARITY     2. /**< Default lacunarity for noise. */


/**
 * @brief Linearly Interpolates x between a and b.
 */
#define LERP(a, b, x)      ( a + x * (b - a) )


/**
 * @brief Structure used for generating noise.
 */
typedef struct perlin_data_s {
   int ndim; /**< Dimension of the noise. */
   unsigned char map[256]; /**< Randomized map of indexes into buffer */
   float buffer[256][3];   /**< Random 256 x 3 buffer */
   /* fractal stuff */
   float H; /**< Not sure. */
   float lacunarity; /**< Not sure. */
   float exponent[TCOD_NOISE_MAX_OCTAVES]; /**< Not sure. */
} perlin_data_t; /**< Internal perlin noise data. */


/*
 * prototypes
 */
/* perlin data handling. */
static perlin_data_t* TCOD_noise_new( int dim, float hurst, float lacunarity );
static void TCOD_noise_delete( perlin_data_t* noise );
/* normalizing. */
static void normalize3( float f[3] );
static void normalize2( float f[2] );
/* noise processing. */
static float lattice3( perlin_data_t *pdata, int ix, float fx,
      int iy, float fy, int iz, float fz );
static float lattice2( perlin_data_t *pdata, int ix, float fx, int iy, float fy );
/* basic perlin noise */
static float TCOD_noise_get3( perlin_data_t* pdata, float f[3] );
static float TCOD_noise_get2( perlin_data_t* pdata, float f[2] );
/* turbulence */
static float TCOD_noise_turbulence3( perlin_data_t* noise, float f[3], int octaves );
static float TCOD_noise_turbulence2( perlin_data_t* noise, float f[2], int octaves );


/**
 * @brief Not sure what it does.
 */
static __inline float lattice3( perlin_data_t *pdata, int ix, float fx, int iy, float fy,
int iz, float fz )
{
   int nIndex;
   float value;

   nIndex = 0;
   nIndex = pdata->map[(nIndex + ix) & 0xFF];
   nIndex = pdata->map[(nIndex + iy) & 0xFF];
   nIndex = pdata->map[(nIndex + iz) & 0xFF];

   value  = pdata->buffer[nIndex][0] * fx;
   value += pdata->buffer[nIndex][1] * fy;
   value += pdata->buffer[nIndex][2] * fz;

   return value;
}


/**
 * @brief Not sure what it does.
 */
static float lattice2( perlin_data_t *pdata, int ix, float fx, int iy, float fy )
{
   int nIndex;
   float value;

   nIndex = 0;
   nIndex = pdata->map[(nIndex + ix) & 0xFF];
   nIndex = pdata->map[(nIndex + iy) & 0xFF];

   value  = pdata->buffer[nIndex][0] * fx;
   value += pdata->buffer[nIndex][1] * fy;

   return value;
}


#define SWAP(a, b, t)      t = a; a = b; b = t /**< Swaps two values. */
#define FLOOR(a) ((int)a - (a < 0 && a != (int)a)) /**< Limits to 0. */
#define CUBIC(a)  ( a * a * (3 - 2*a) ) /**< Does cubic filtering. */


/**
 * @brief Normalizes a 3d vector.
 *
 *    @param f Vector to normalize.
 */
static void normalize3( float f[3] )
{
   float magnitude;

   magnitude = 1. / sqrtf(f[0]*f[0] + f[1]*f[1] + f[2]*f[2]);
   f[0] *= magnitude;
   f[1] *= magnitude;
   f[2] *= magnitude;
}


/**
 * @brief Normalizes a 2d vector.
 *
 *    @param f Vector to normalize.
 */
static void normalize2( float f[2] )
{
   float magnitude;

   magnitude = 1. / sqrtf(f[0]*f[0] + f[1]*f[1]);
   f[0] *= magnitude;
   f[1] *= magnitude;
}


/**
 * @brief Creates a new perlin noise generator.
 *
 *    @param dim Dimension of the noise.
 *    @param hurst
 *    @param lacunarity
 */
static perlin_data_t* TCOD_noise_new( int dim, float hurst, float lacunarity )
{
   perlin_data_t *pdata;
   int i, j;
   unsigned char tmp;
   float f;

   /* Create the data. */
   pdata = calloc(sizeof(perlin_data_t),1);
   pdata->ndim = dim;

   /* Create the buffer and map. */
   if (dim == 3) {
      for(i=0; i<256; i++) {
         pdata->map[i] = (unsigned char)i;
         pdata->buffer[i][0] = RNGF()-0.5;
         pdata->buffer[i][1] = RNGF()-0.5;
         pdata->buffer[i][2] = RNGF()-0.5;
         normalize3(pdata->buffer[i]);
      }
   }
   else if (dim == 2) {
      for(i=0; i<256; i++) {
         pdata->map[i] = (unsigned char)i;
         pdata->buffer[i][0] = RNGF()-0.5;
         pdata->buffer[i][1] = RNGF()-0.5;
         normalize2(pdata->buffer[i]);
      }
   }
   else {
      i = 0;
   }

   while(--i) {
      j = RNG(0, 255);
      SWAP(pdata->map[i], pdata->map[j], tmp);
   }

   f = 1.;
   pdata->H = hurst;
   pdata->lacunarity = lacunarity;
   for(i=0; i<TCOD_NOISE_MAX_OCTAVES; i++) {
      /*exponent[i] = powf(f, -H); */
      pdata->exponent[i] = 1. / f;
      f *= lacunarity;
   }

   return pdata;
}


/**
 * @brief Gets some 3D Perlin noise from the data.
 *
 * Somewhat optimized for speed, probably can't get optimized much more.
 *
 *    @param pdata Perlin data to use.
 *    @param f Position of the noise to get.
 */
static float TCOD_noise_get3( perlin_data_t* pdata, float f[3] )
{
   int n[3] __attribute__ ((aligned (32))); /* Indexes to pass to lattice function */
   float r[3] __attribute__ ((aligned (32))); /* Remainders to pass to lattice function */
   float w[3] __attribute__ ((aligned (32))); /* Cubic values to pass to interpolation function */
   float value;
   float v[8] __attribute__ ((aligned (32)));

   n[0] = (int)f[0];
   n[1] = (int)f[1];
   n[2] = (int)f[2];

   r[0] = f[0] - n[0];
   r[1] = f[1] - n[1];
   r[2] = f[2] - n[2];

   w[0] = CUBIC(r[0]);
   w[1] = CUBIC(r[1]);
   w[2] = CUBIC(r[2]);

   /*
    * This is the big ugly bit in dire need of optimization
    */
   v[0] = lattice3(pdata, n[0],   r[0],   n[1],   r[1],   n[2],   r[2]);
   v[1] = lattice3(pdata, n[0]+1, r[0]-1, n[1],   r[1],   n[2],   r[2]);
   v[2] = lattice3(pdata, n[0],   r[0],   n[1]+1, r[1]-1, n[2],   r[2]);
   v[3] = lattice3(pdata, n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2],   r[2]);
   v[4] = lattice3(pdata, n[0],   r[0],   n[1],   r[1],   n[2]+1, r[2]-1);
   v[5] = lattice3(pdata, n[0]+1, r[0]-1, n[1],   r[1],   n[2]+1, r[2]-1);
   v[6] = lattice3(pdata, n[0],   r[0],   n[1]+1, r[1]-1, n[2]+1, r[2]-1);
   v[7] = lattice3(pdata, n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2]+1, r[2]-1);
   value = LERP(
         LERP(
            LERP(v[0], v[1], w[0]),
            LERP(v[2], v[3], w[0]),
            w[1]
            ),
         LERP(
            LERP(v[4], v[5], w[0]),
            LERP(v[6], v[7], w[0]),
            w[1]
            ),
         w[2]
         );

   return CLAMP(-0.99999f, 0.99999f, value);
}


/**
 * @brief Gets some 2D Perlin noise from the data.
 *
 * Somewhat optimized for speed, probably can't get optimized much more.
 *
 *    @param pdata Perlin data to use.
 *    @param f Position of the noise to get.
 */
static float TCOD_noise_get2( perlin_data_t* pdata, float f[2] )
{
   int n[2] __attribute__ ((aligned (32))); /* Indexes to pass to lattice function */
   float r[2] __attribute__ ((aligned (32))); /* Remainders to pass to lattice function */
   float w[2] __attribute__ ((aligned (32))); /* Cubic values to pass to interpolation function */
   float value __attribute__ ((aligned (32)));
   float v[4] __attribute__ ((aligned (32)));

   n[0] = FLOOR(f[0]);
   n[1] = FLOOR(f[1]);

   r[0] = f[0] - n[0];
   r[1] = f[1] - n[1];

   w[0] = CUBIC(r[0]);
   w[1] = CUBIC(r[1]);

   /*
    * Much faster in 2d.
    */
   v[0] = lattice2(pdata,n[0],   r[0],   n[1],   r[1]);
   v[1] = lattice2(pdata,n[0]+1, r[0]-1, n[1],   r[1]);
   v[2] = lattice2(pdata,n[0],   r[0],   n[1]+1, r[1]-1);
   v[3] = lattice2(pdata,n[0]+1, r[0]-1, n[1]+1, r[1]-1);
   value = LERP(LERP(v[0], v[1], w[0]),
         LERP(v[2], v[3], w[0]),
         w[1]);

   return CLAMP(-0.99999f, 0.99999f, value);
}


/**
 * @brief Gets 3d Turbulence noise for a position.
 *
 *    @param noise Perlin data to generate noise from.
 *    @param f Position of the noise.
 *    @param octaves Octaves to use.
 *    @return The noise level at the position.
 */
static float TCOD_noise_turbulence3( perlin_data_t* noise, float f[3], int octaves )
{
   float tf[3];
   perlin_data_t *pdata=(perlin_data_t *)noise;
   /* Initialize locals */
   float value = 0;
   int i;

   tf[0] = f[0];
   tf[1] = f[1];
   tf[2] = f[2];

   /* Inner loop of spectral construction, where the fractal is built */
   for(i=0; i<octaves; i++)
   {
      value += ABS(TCOD_noise_get3(noise,tf)) * pdata->exponent[i];
      tf[0] *= pdata->lacunarity;
      tf[1] *= pdata->lacunarity;
      tf[2] *= pdata->lacunarity;
   }

   return CLAMP(-0.99999f, 0.99999f, value);
}


/**
 * @brief Gets 2d Turbulence noise for a position.
 *
 *    @param noise Perlin data to generate noise from.
 *    @param f Position of the noise.
 *    @param octaves Octaves to use.
 *    @return The noise level at the position.
 */
static float TCOD_noise_turbulence2( perlin_data_t* noise, float f[2], int octaves )
{
   float tf[2];
   perlin_data_t *pdata=(perlin_data_t *)noise;
   /* Initialize locals */
   float value = 0;
   int i;

   tf[0] = f[0];
   tf[1] = f[1];

   /* Inner loop of spectral construction, where the fractal is built */
   for(i=0; i<octaves; i++)
   {
      value += ABS(TCOD_noise_get2(noise,tf)) * pdata->exponent[i];
      tf[0] *= pdata->lacunarity;
      tf[1] *= pdata->lacunarity;
   }

   return CLAMP(-0.99999f, 0.99999f, value);
}


/**
 * @brief Frees some noise data.
 *
 *    @param noise Noise data to free.
 */
void TCOD_noise_delete( perlin_data_t* noise )
{
   free(noise);
}


/**
 * @brief Generates radar interference.
 *
 *    @param w Width to generate.
 *    @param h Height to generate.
 *    @param rug Rugosity of the interference.
 *    @return The map generated.
 */
float* noise_genRadarInt( const int w, const int h, float rug )
{
   int x, y;
   float f[2];
   int octaves;
   float hurst;
   float lacunarity;
   perlin_data_t* noise;
   float *map;
   float value;

   /* pretty default values */
   octaves     = 3;
   hurst       = TCOD_NOISE_DEFAULT_HURST;
   lacunarity  = TCOD_NOISE_DEFAULT_LACUNARITY;

   /* create noise and data */
   noise       = TCOD_noise_new( 2, hurst, lacunarity );
   map         = malloc(sizeof(float)*w*h);
   if (map == NULL) {
      WARN("Out of memory!");
      return NULL;
   }

   /* Start to create the nebula */
   for (y=0; y<h; y++) {

      f[1] = rug * (float)y / (float)h;
      for (x=0; x<w; x++) {

         f[0] = rug * (float)x / (float)w;

         /* Get the 2d noise. */
         value = TCOD_noise_get2( noise, f );

         /* Set the value to [0,1]. */
         map[y*w + x] = (value + 1.) / 2.;
      }
   }

   /* Clean up */
   TCOD_noise_delete( noise );

   /* Results */
   return map;
}


/**
 * @brief Generates a 3d nebula map.
 *
 *    @param w Width of the map.
 *    @param h Height of the map.
 *    @param n Number of slices of the map (2d planes).
 *    @param rug Rugosity of the map.
 *    @return The map generated.
 */
float* noise_genNebulaMap( const int w, const int h, const int n, float rug )
{
   int x, y, z;
   float f[3];
   int octaves;
   float hurst;
   float lacunarity;
   perlin_data_t* noise;
   float *nebula;
   float value;
   float zoom;
   float max;
   unsigned int *t, s;

   /* pretty default values */
   octaves     = 3;
   hurst       = TCOD_NOISE_DEFAULT_HURST;
   lacunarity  = TCOD_NOISE_DEFAULT_LACUNARITY;
   zoom        = rug * ((float)h/768.)*((float)w/1024.);

   /* create noise and data */
   noise       = TCOD_noise_new( 3, hurst, lacunarity );
   nebula     = malloc(sizeof(float)*w*h*n);
   if (nebula == NULL) {
      WARN("Out of memory!");
      return NULL;
   }

   /* Some debug information and time setting */
   s = SDL_GetTicks();
   t = malloc(sizeof(unsigned int)*n);
   DEBUG("Generating Nebula of size %dx%dx%d", w, h, n);

   /* Start to create the nebula */
   max = 0.;
   for (z=0; z<n; z++) {

      f[2] = zoom * (float)z / (float)n;

      for (y=0; y<h; y++) {

         f[1] = zoom * (float)y / (float)h;

         for (x=0; x<w; x++) {

            f[0] = zoom * (float)x / (float)w;

            value = TCOD_noise_turbulence3( noise, f, octaves );
            if (max < value) max = value;

            nebula[z*w*h + y*w + x] = value;
         }
      }

      /* More time magic debug */
      t[z] = SDL_GetTicks();
      DEBUG("   Layer %d/%d generated in %d ms", z+1, n,
            (z>0) ? t[z] - t[z-1] : t[z] - s );
   }

   /* Post filtering */
   value = 1. - max;
   for (z=0; z<n; z++)
      for (y=0; y<h; y++)
         for (x=0; x<w; x++)
            nebula[z*w*h + y*w + x] += value;

   /* Clean up */
   TCOD_noise_delete( noise );

   /* Results */
   DEBUG("Nebula Generated in %d ms", SDL_GetTicks() - s );
   return nebula;
}


/**
 * @brief Generates tiny nebula puffs
 *
 *    @param w Width of the puff to generate.
 *    @param h Height of the puff to generate.
 *    @param rug Rugosity of the puff.
 *    @return The puff generated.
 */
float* noise_genNebulaPuffMap( const int w, const int h, float rug )
{
   int x,y, hw,hh;
   float d;
   float f[2];
   int octaves;
   float hurst;
   float lacunarity;
   perlin_data_t* noise;
   float *nebula;
   float value;
   float zoom;
   float max;

   /* pretty default values */
   octaves     = 3;
   hurst       = TCOD_NOISE_DEFAULT_HURST;
   lacunarity  = TCOD_NOISE_DEFAULT_LACUNARITY;
   zoom        = rug;

   /* create noise and data */
   noise       = TCOD_noise_new( 2, hurst, lacunarity );
   nebula     = malloc(sizeof(float)*w*h);
   if (nebula == NULL) {
      WARN("Out of memory!");
      return NULL;
   }

   /* Start to create the nebula */
   max   = 0.;
   hw    = w/2;
   hh    = h/2;
   d     = (float)MIN(hw,hh);
   for (y=0; y<h; y++) {

      f[1] = zoom * (float)y / (float)h;
      for (x=0; x<w; x++) {

         f[0] = zoom * (float)x / (float)w;

         /* Get the 2d noise. */
         value = TCOD_noise_turbulence2( noise, f, octaves );

         /* Make value also depend on distance from center */
         value *= (d - 1. - sqrtf( (float)((x-hw)*(x-hw) + (y-hh)*(y-hh)) )) / d;
         if (value < 0.)
            value = 0.;

         /* Cap at maximum. */
         if (max < value)
            max = value;

         /* Set the value. */
         nebula[y*w + x] = value;
      }
   }

   /* Clean up */
   TCOD_noise_delete( noise );

   /* Results */
   return nebula;
}


