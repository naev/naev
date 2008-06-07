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


#include "perlin.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "naev.h"
#include "log.h"
#include "rng.h"
#include "nfile.h"


#define TCOD_NOISE_MAX_OCTAVES            128   
#define TCOD_NOISE_DEFAULT_HURST          0.5
#define TCOD_NOISE_DEFAULT_LACUNARITY     2.


#define LERP(a, b, x)      ( a + x * (b - a) )
#define CLAMP(a, b, x)     ((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))


typedef void *TCOD_noise_t;


/*
 * Used internally
 */
typedef struct {
   unsigned char map[256]; /* Randomized map of indexes into buffer */
   float buffer[256][3];   /* Random 256 x 3 buffer */
   /* fractal stuff */
   float H;
   float lacunarity;
   float exponent[TCOD_NOISE_MAX_OCTAVES];
} perlin_data_t;


/*
 * prototypes
 */
static perlin_data_t* TCOD_noise_new( float hurst, float lacunarity );
/* basic perlin noise */
static float TCOD_noise_get( perlin_data_t* pdata, float *f );
/* fractional brownian motion */
/* turbulence */
static float TCOD_noise_turbulence( perlin_data_t* noise, float *f, int octaves );
static void TCOD_noise_delete(perlin_data_t* noise);


static float lattice( perlin_data_t *pdata, int ix, float fx, int iy, float fy, int iz, float fz )
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

#define SWAP(a, b, t)      t = a; a = b; b = t
#define FLOOR(a) ((int)a - (a < 0 && a != (int)a))
#define CUBIC(a)  ( a * a * (3 - 2*a) )

static void normalize(float f[3])
{
   float magnitude;

   magnitude = 1. / sqrtf(f[0]*f[0] + f[1]*f[1] + f[2]*f[2]);
   f[0] *= magnitude;
   f[1] *= magnitude;
   f[2] *= magnitude;
}


static perlin_data_t* TCOD_noise_new( float hurst, float lacunarity )
{
   perlin_data_t *pdata=(perlin_data_t *)calloc(sizeof(perlin_data_t),1);
   int i, j;
   unsigned char tmp;
   float f = 1;
   for(i=0; i<256; i++)
   {
      pdata->map[i] = (unsigned char)i;
      pdata->buffer[i][0] = RNGF()-0.5;
      pdata->buffer[i][1] = RNGF()-0.5;
      pdata->buffer[i][2] = RNGF()-0.5;
      normalize(pdata->buffer[i]);
   }

   while(--i)
   {
      j = RNG(0, 255);
      SWAP(pdata->map[i], pdata->map[j], tmp);
   }

   pdata->H = hurst;
   pdata->lacunarity = lacunarity;
   for(i=0; i<TCOD_NOISE_MAX_OCTAVES; i++)
   {
      /*exponent[i] = powf(f, -H); */
      pdata->exponent[i] = 1. / f;
      f *= lacunarity;
   }
   return (TCOD_noise_t)pdata;
}

static float TCOD_noise_get( perlin_data_t* pdata, float *f )
{
   int n[3]; /* Indexes to pass to lattice function */
   float r[3]; /* Remainders to pass to lattice function */
   float w[3]; /* Cubic values to pass to interpolation function */
   float value;

   n[0] = FLOOR(f[0]);
   n[1] = FLOOR(f[1]);
   n[2] = FLOOR(f[2]);

   r[0] = f[0] - n[0];
   r[1] = f[1] - n[1];
   r[2] = f[2] - n[2];

   w[0] = CUBIC(r[0]);
   w[1] = CUBIC(r[1]);
   w[2] = CUBIC(r[2]);

   /*
    * This is the big ugly bit in dire need of optimization
    */
   value = LERP(LERP(LERP(lattice(pdata,n[0], r[0], n[1], r[1], n[2], r[2]),
               lattice(pdata,n[0]+1, r[0]-1, n[1], r[1], n[2], r[2]),
               w[0]),
            LERP(lattice(pdata,n[0], r[0], n[1]+1, r[1]-1, n[2], r[2]),
               lattice(pdata,n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2], r[2]),
               w[0]),
            w[1]),
         LERP(LERP(lattice(pdata,n[0], r[0], n[1], r[1], n[2]+1, r[2]-1),
               lattice(pdata,n[0]+1, r[0]-1, n[1], r[1], n[2]+1, r[2]-1),
               w[0]),
            LERP(lattice(pdata,n[0], r[0], n[1]+1, r[1]-1, n[2]+1, r[2]-1),
               lattice(pdata,n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2]+1, r[2]-1),
               w[0]),
            w[1]),
         w[2]);

   return CLAMP(-0.99999f, 0.99999f, value);
}

static float TCOD_noise_turbulence( perlin_data_t* noise, float *f, int octaves )
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
      value += ABS(TCOD_noise_get(noise,tf)) * pdata->exponent[i];
      tf[0] *= pdata->lacunarity;
      tf[1] *= pdata->lacunarity;
      tf[2] *= pdata->lacunarity;
   }

   return CLAMP(-0.99999f, 0.99999f, value);
}

void TCOD_noise_delete(perlin_data_t* noise) {
   free((perlin_data_t *)noise);
}


/*
 * Generates a 3d nebulae map of dimensions w,h,n with ruggedness rug
 */
float* noise_genNebulaeMap( const int w, const int h, const int n, float rug )
{
   int x, y, z;
   float f[3];
   int octaves;
   float hurst;
   float lacunarity;
   perlin_data_t* noise;
   float *nebulae;
   float value;
   unsigned int *t, s;

   /* pretty default values */
   octaves = 3;
   hurst = TCOD_NOISE_DEFAULT_HURST;
   lacunarity = TCOD_NOISE_DEFAULT_LACUNARITY;

   /* create noise and data */
   noise = TCOD_noise_new( hurst, lacunarity );
   nebulae = malloc(sizeof(float)*w*h*n);
   if (nebulae == NULL) {
      WARN("Out of memory!");
      return NULL;
   }

   /* Some debug information and time setting */
   s = SDL_GetTicks();
   t = malloc(sizeof(unsigned int)*n);
   DEBUG("Generating Nebulae of size %dx%dx%d", w, h, n);

   /* Start to create the nebulae */
   f[2] = 0.;
   for (z=0; z<n; z++) {
      for (y=0; y<h; y++) {

         f[1] = rug * (float)y / (float)h;

         for (x=0; x<w; x++) {

            f[0] = rug * (float)x / (float)w;

            value = TCOD_noise_turbulence( noise, f, octaves );

            value = value + 0.3;
            nebulae[z*w*h + y*w + x] = (value < 1.) ? value : 1.;
         }
      }
      f[2] += 0.01;

      /* More time magic debug */
      t[z] = SDL_GetTicks();
      DEBUG("   Layer %d/%d generated in %d ms", z+1, n,
            (z>0) ? t[z] - t[z-1] : t[z] - s );
   }

   /* Clean up */
   TCOD_noise_delete( noise );

   /* Results */
   DEBUG("Nebulae Generated in %d ms", SDL_GetTicks() - s );
   return nebulae;
}


/*
 * Generates a SDL_Surface from a 2d nebulae map
 */
SDL_Surface* noise_surfaceFromNebulaeMap( float* map, const int w, const int h )
{
   int i;
   SDL_Surface *sur;
   uint32_t *pix;
   double c;

   sur = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, 32, RGBAMASK );
   pix = sur->pixels;

   /* convert from mapping to actual colours */
   SDL_LockSurface( sur );
   for (i=0; i<h*w; i++) {
      c = map[i];
      pix[i] = RMASK + BMASK + GMASK + (AMASK & (uint32_t)(AMASK*c));
   }
   SDL_UnlockSurface( sur );

   return sur;
}

