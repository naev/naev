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
#include "nstring.h"

#include "SDL.h"
#include "SDL_thread.h"
#include "threadpool.h"

#include "log.h"
#include "rng.h"
#include "nfile.h"


#define SIMPLEX_SCALE 0.5f


/**
 * @brief Linearly Interpolates x between a and b.
 */
#define LERP(a, b, x)      ( a + x * (b - a) )


/**
 * @brief Structure used for generating noise.
 */
struct perlin_data_s {
   int ndim; /**< Dimension of the noise. */
   unsigned char map[256]; /**< Randomized map of indexes into buffer */
   float buffer[256][3];   /**< Random 256 x 3 buffer */
   /* fractal stuff */
   float H; /**< Not sure. */
   float lacunarity; /**< Not sure. */
   float exponent[NOISE_MAX_OCTAVES]; /**< Not sure. */
};


/**
 * @brief Threading stuff.
 */
typedef struct thread_args_ {
   int z; /**< Z level working on. */
   float zoom; /**< Zoom level of detail. */
   int n; /**< Number of layers to generate. */
   int h; /**< Height. */
   int w; /**< Width. */
   perlin_data_t *noise; /**< Parent noise. */
   int octaves; /**< Octave parameters. */
   float *max; /**< Maximum value. */
   float *nebula; /**< Nebula loading into. */
} thread_args;


/*
 * prototypes
 */
/* normalizing. */
static void normalize3( float f[3] );
static void normalize2( float f[2] );
/* noise processing. */
static float lattice3( perlin_data_t *pdata, int ix, float fx,
      int iy, float fy, int iz, float fz );
static float lattice2( perlin_data_t *pdata, int ix, float fx, int iy, float fy );
static float lattice1( perlin_data_t *pdata, int ix, float fx );
/*Threading */
static int noise_genNebulaMap_thread( void *data );


/**
 * @brief Not sure what it does.
 */
static float lattice3( perlin_data_t *pdata, int ix, float fx, int iy, float fy,
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


/**
 * @brief Not sure what it does.
 */
static float lattice1( perlin_data_t *pdata, int ix, float fx )
{
   int nIndex;
   float value;

   nIndex = 0;
   nIndex = pdata->map[(nIndex + ix) & 0xFF];

   value  = pdata->buffer[nIndex][0] * fx;

   return value;
}


#define SWAP(a, b, t)      (t) = (a); (a) = (b); (b) = (t) /**< Swaps two values. */
#define FLOOR(a)           ((int)(a) - ((a) < 0 && (a) != (int)(a))) /**< Limits to 0. */
#define CUBIC(a)           ( (a) * (a) * (3 - 2*(a)) ) /**< Does cubic filtering. */


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
perlin_data_t* noise_new( int dim, float hurst, float lacunarity )
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
      for (i=0; i<256; i++) {
         pdata->map[i] = (unsigned char)i;
         pdata->buffer[i][0] = 1.;
      }
   }

   while (--i) {
      j = RNG(0, 255);
      SWAP(pdata->map[i], pdata->map[j], tmp);
   }

   f = 1.;
   pdata->H = hurst;
   pdata->lacunarity = lacunarity;
   for(i=0; i<NOISE_MAX_OCTAVES; i++) {
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
float noise_get3( perlin_data_t* pdata, float f[3] )
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
float noise_get2( perlin_data_t* pdata, float f[2] )
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
 * @brief Gets some 1D Perlin noise from the data.
 *
 * Somewhat optimized for speed, probably can't get optimized much more.
 *
 *    @param pdata Perlin data to use.
 *    @param f Position of the noise to get.
 */
float noise_get1( perlin_data_t* pdata, float f[1] )
{
   int n[1] __attribute__ ((aligned (32))); /* Indexes to pass to lattice function */
   float r[1] __attribute__ ((aligned (32))); /* Remainders to pass to lattice function */
   float w[1] __attribute__ ((aligned (32))); /* Cubic values to pass to interpolation function */
   float value __attribute__ ((aligned (32)));
   float v[2] __attribute__ ((aligned (32)));

   n[0] = FLOOR(f[0]);

   r[0] = f[0] - n[0];

   w[0] = CUBIC(r[0]);

   v[0] = lattice1(pdata,n[0],   r[0]   );
   v[1] = lattice1(pdata,n[0]+1, r[0]-1 );
   value = LERP( v[0], v[1], w[0] );

   return CLAMP(-0.99999f, 0.99999f, value);
}


/**
 * @brief Gets 3d Turbulence noise for a position.
 *
 *    @param pdata Perlin data to generate noise from.
 *    @param f Position of the noise.
 *    @param octaves Octaves to use.
 *    @return The noise level at the position.
 */
float noise_turbulence3( perlin_data_t* pdata, float f[3], int octaves )
{
   float tf[3];
   /* Initialize locals */
   float value = 0;
   int i;

   tf[0] = f[0];
   tf[1] = f[1];
   tf[2] = f[2];

   /* Inner loop of spectral construction, where the fractal is built */
   for(i=0; i<octaves; i++)
   {
      value += ABS(noise_get3(pdata,tf)) * pdata->exponent[i];
      tf[0] *= pdata->lacunarity;
      tf[1] *= pdata->lacunarity;
      tf[2] *= pdata->lacunarity;
   }

   return CLAMP(-0.99999f, 0.99999f, value);
}


/**
 * @brief Gets 2d Turbulence noise for a position.
 *
 *    @param pdata Perlin data to generate noise from.
 *    @param f Position of the noise.
 *    @param octaves Octaves to use.
 *    @return The noise level at the position.
 */
float noise_turbulence2( perlin_data_t* pdata, float f[2], int octaves )
{
   float tf[2];
   /* Initialize locals */
   float value = 0;
   int i;

   tf[0] = f[0];
   tf[1] = f[1];

   /* Inner loop of spectral construction, where the fractal is built */
   for(i=0; i<octaves; i++)
   {
      value += ABS(noise_get2(pdata,tf)) * pdata->exponent[i];
      tf[0] *= pdata->lacunarity;
      tf[1] *= pdata->lacunarity;
   }

   return CLAMP(-0.99999f, 0.99999f, value);
}


/**
 * @brief Gets 1d Turbulence noise for a position.
 *
 *    @param pdata Perlin data to generate noise from.
 *    @param f Position of the noise.
 *    @param octaves Octaves to use.
 *    @return The noise level at the position.
 */
float noise_turbulence1( perlin_data_t* pdata, float f[1], int octaves )
{
   float tf[1];
   /* Initialize locals */
   float value = 0;
   int i;

   tf[0] = f[0];

   /* Inner loop of spectral construction, where the fractal is built */
   for(i=0; i<octaves; i++)
   {
      value += ABS(noise_get1(pdata,tf)) * pdata->exponent[i];
      tf[0] *= pdata->lacunarity;
   }

   return CLAMP(-0.99999f, 0.99999f, value);
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
 *    @param noise Noise data to free.
 */
void noise_delete( perlin_data_t* pdata )
{
   free(pdata);
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
   float hurst;
   float lacunarity;
   perlin_data_t* noise;
   float *map;
   float value;

   /* pretty default values */
   hurst       = NOISE_DEFAULT_HURST;
   lacunarity  = NOISE_DEFAULT_LACUNARITY;

   /* create noise and data */
   noise       = noise_new( 2, hurst, lacunarity );
   map         = malloc(sizeof(float)*w*h);
   if (map == NULL) {
      noise_delete( noise );
      WARN("Out of memory!");
      return NULL;
   }

   /* Start to create the nebula */
   for (y=0; y<h; y++) {

      f[1] = rug * (float)y / (float)h;
      for (x=0; x<w; x++) {

         f[0] = rug * (float)x / (float)w;

         /* Get the 2d noise. */
         value = noise_get2( noise, f );

         /* Set the value to [0,1]. */
         map[y*w + x] = (value + 1.) / 2.;
      }
   }

   /* Clean up */
   noise_delete( noise );

   /* Results */
   return map;
}


/**
 * @brief Thread worker for generating nebula stuff.
 *
 *    @param data Data to pass.
 */
static int noise_genNebulaMap_thread( void *data )
{
   thread_args *args = (thread_args*) data;
   float f[3];
   float value;
   int y, x;
   float max;

   /* Generate the layer. */
   max = 0;
   f[2] = args->zoom * (float)args->z / (float)args->n;

   for (y=0; y<args->h; y++) {
      f[1] = args->zoom * (float)y / (float)args->h;

      for (x=0; x<args->w; x++) {
         f[0] = args->zoom * (float)x / (float)args->w;

         value = noise_turbulence3( args->noise, f, args->octaves );
         if (max < value)
            max = value;

         args->nebula[args->z * args->w * args->h + y * args->w + x] = value;
       }
   }

   /* Set up output. */
   *args->max = max;

   /* Clean up. */
   free( args );
   return 0;
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
   int x, y, z, i;
   int octaves;
   float hurst;
   float lacunarity;
   perlin_data_t* noise;
   float *nebula;
   float value;
   float zoom;
   float *_max;
   float max;
   unsigned int s;
   thread_args *args;
   ThreadQueue *vpool;

   /* pretty default values */
   octaves     = 3;
   hurst       = NOISE_DEFAULT_HURST;
   lacunarity  = NOISE_DEFAULT_LACUNARITY;
   zoom        = rug * ((float)h/768.)*((float)w/1024.);

   /* create noise and data */
   noise      = noise_new( 3, hurst, lacunarity );
   nebula     = malloc(sizeof(float)*w*h*n);
   if (nebula == NULL) {
      noise_delete( noise );
      WARN("Out of memory!");
      return NULL;
   }

   /* Some debug information and time setting */
   s = SDL_GetTicks();
   DEBUG("Generating Nebula of size %dx%dx%d", w, h, n);

   /* Prepare for generation. */
   _max        = malloc( sizeof(float) * n );

   /* Initialize vpool */
   vpool = vpool_create();

   /* Start to create the nebula */
   for (z=0; z<n; z++) {
      /* Make ze arguments! */
      args     = malloc( sizeof(thread_args) );
      args->z  = z;
      args->zoom = zoom;
      args->n  = n;
      args->h  = h;
      args->w  = w;
      args->noise = noise;
      args->octaves = octaves;
      args->max = &_max[z];
      args->nebula = nebula;

      /* Launch ze thread. */
      vpool_enqueue( vpool, noise_genNebulaMap_thread, args );
   }

   /* Wait for threads to signal completion. */
   vpool_wait( vpool );
   max = 0.;
   for (i=0; i<n; i++) {
      if (_max[i]>max)
         max = _max[i];
   }

   /* Post filtering */
   value = 1. - max;
   for (z=0; z<n; z++)
      for (y=0; y<h; y++)
         for (x=0; x<w; x++)
            nebula[z*w*h + y*w + x] += value;

   /* Clean up */
   noise_delete( noise );
   free(_max);

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
   hurst       = NOISE_DEFAULT_HURST;
   lacunarity  = NOISE_DEFAULT_LACUNARITY;
   zoom        = rug;

   /* create noise and data */
   noise       = noise_new( 2, hurst, lacunarity );
   nebula      = malloc(sizeof(float)*w*h);
   if (nebula == NULL) {
      noise_delete( noise );
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
         value = noise_turbulence2( noise, f, octaves );

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
   noise_delete( noise );

   /* Results */
   return nebula;
}


