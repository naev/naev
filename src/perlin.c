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


#define NEBULAE_Z                         32


#define TCOD_NOISE_MAX_OCTAVES            128   
#define TCOD_NOISE_MAX_DIMENSIONS         4
#define TCOD_NOISE_DEFAULT_HURST          0.5
#define TCOD_NOISE_DEFAULT_LACUNARITY     2.


#define LERP(a, b, x)      ( a + x * (b - a) )
#define CLAMP(a, b, x)     ((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))


typedef void *TCOD_noise_t;


/*
 * Used internally
 */
typedef struct {
	int ndim;
	unsigned char map[256]; /* Randomized map of indexes into buffer */
	float buffer[256][TCOD_NOISE_MAX_DIMENSIONS]; 	/* Random 256 x ndim buffer */
	/* fractal stuff */
	float H;
	float lacunarity;
	float exponent[TCOD_NOISE_MAX_OCTAVES];
} perlin_data_t;


/*
 * prototypes
 */
static float* genNebulaeMap( const int w, const int h, const int n, float rug );
SDL_Surface* surfaceFromNebulaeMap( float* map, const int w, const int h );
static TCOD_noise_t TCOD_noise_new(int dimensions, float hurst, float lacunarity );
/* basic perlin noise */
static float TCOD_noise_get( TCOD_noise_t noise, float *f );
/* fractional brownian motion */
/* static float TCOD_noise_fbm( TCOD_noise_t noise, float *f, float octaves ); */
/* turbulence */
static float TCOD_noise_turbulence( TCOD_noise_t noise, float *f, float octaves );
static void TCOD_noise_delete(TCOD_noise_t noise);


static float lattice( perlin_data_t *pdata, int ix, float fx, int iy, float fy, int iz, float fz, int iw, float fw)
{
	int n[4] = {ix, iy, iz, iw};
	float f[4] = {fx, fy, fz, fw};
	int nIndex = 0;
	int i;
	float value = 0;

	for(i=0; i<pdata->ndim; i++)
		nIndex = pdata->map[(nIndex + n[i]) & 0xFF];
	for(i=0; i<pdata->ndim; i++)
		value += pdata->buffer[nIndex][i] * f[i];
	return value;
}

#define DEFAULT_SEED 0x15687436
#define DELTA				1e-6f
#define SWAP(a, b, t)		t = a; a = b; b = t

#define FLOOR(a) ((int)a - (a < 0 && a != (int)a))
#define CUBIC(a)	( a * a * (3 - 2*a) )

static void normalize(perlin_data_t *pdata, float *f)
{
	float magnitude = 0;
	int i;
	for(i=0; i<pdata->ndim; i++)
		magnitude += f[i]*f[i];
	magnitude = 1 / sqrtf(magnitude);
	for(i=0; i<pdata->ndim; i++)
		f[i] *= magnitude;
}


static TCOD_noise_t TCOD_noise_new(int ndim, float hurst, float lacunarity )
{
	perlin_data_t *pdata=(perlin_data_t *)calloc(sizeof(perlin_data_t),1);
	int i, j;
	unsigned char tmp;
	float f = 1;
	pdata->ndim = ndim;
	for(i=0; i<256; i++)
	{
		pdata->map[i] = (unsigned char)i;
		for(j=0; j<pdata->ndim; j++)
			pdata->buffer[i][j] = RNGF()-0.5;
		normalize(pdata,pdata->buffer[i]);
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
		pdata->exponent[i] = 1.0f / f;
		f *= lacunarity;
	}
	return (TCOD_noise_t)pdata;
}

static float TCOD_noise_get( TCOD_noise_t noise, float *f )
{
	perlin_data_t *pdata=(perlin_data_t *)noise;
	int n[TCOD_NOISE_MAX_DIMENSIONS];			/* Indexes to pass to lattice function */
	int i;
	float r[TCOD_NOISE_MAX_DIMENSIONS];		/* Remainders to pass to lattice function */
	float w[TCOD_NOISE_MAX_DIMENSIONS];		/* Cubic values to pass to interpolation function */
	float value;

	for(i=0; i<pdata->ndim; i++)
	{
		n[i] = FLOOR(f[i]);
		r[i] = f[i] - n[i];
		w[i] = CUBIC(r[i]);
	}

	switch(pdata->ndim)
	{
		case 1:
			value = LERP(lattice(pdata,n[0], r[0],0,0,0,0,0,0),
						  lattice(pdata,n[0]+1, r[0]-1,0,0,0,0,0,0),
						  w[0]);
			break;
		case 2:
			value = LERP(LERP(lattice(pdata,n[0], r[0], n[1], r[1],0,0,0,0),
							   lattice(pdata,n[0]+1, r[0]-1, n[1], r[1],0,0,0,0),
							   w[0]),
						  LERP(lattice(pdata,n[0], r[0], n[1]+1, r[1]-1,0,0,0,0),
							   lattice(pdata,n[0]+1, r[0]-1, n[1]+1, r[1]-1,0,0,0,0),
							   w[0]),
						  w[1]);
			break;
		case 3:
			value = LERP(LERP(LERP(lattice(pdata,n[0], r[0], n[1], r[1], n[2], r[2],0,0),
									lattice(pdata,n[0]+1, r[0]-1, n[1], r[1], n[2], r[2],0,0),
									w[0]),
							   LERP(lattice(pdata,n[0], r[0], n[1]+1, r[1]-1, n[2], r[2],0,0),
									lattice(pdata,n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2], r[2],0,0),
									w[0]),
							   w[1]),
						  LERP(LERP(lattice(pdata,n[0], r[0], n[1], r[1], n[2]+1, r[2]-1,0,0),
									lattice(pdata,n[0]+1, r[0]-1, n[1], r[1], n[2]+1, r[2]-1,0,0),
									w[0]),
							   LERP(lattice(pdata,n[0], r[0], n[1]+1, r[1]-1, n[2]+1, r[2]-1,0,0),
									lattice(pdata,n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2]+1, r[2]-1,0,0),
									w[0]),
							   w[1]),
						  w[2]);
			break;
		case 4:
		default:
			value = LERP(LERP(LERP(LERP(lattice(pdata,n[0], r[0], n[1], r[1], n[2], r[2], n[3], r[3]),
										 lattice(pdata,n[0]+1, r[0]-1, n[1], r[1], n[2], r[2], n[3], r[3]),
										 w[0]),
									LERP(lattice(pdata,n[0], r[0], n[1]+1, r[1]-1, n[2], r[2], n[3], r[3]),
										 lattice(pdata,n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2], r[2], n[3], r[3]),
										 w[0]),
									w[1]),
									LERP(LERP(lattice(pdata,n[0], r[0], n[1], r[1], n[2]+1, r[2]-1, n[3], r[3]),
										 lattice(pdata,n[0]+1, r[0]-1, n[1], r[1], n[2]+1, r[2]-1, n[3], r[3]),
										 w[0]),
									LERP(lattice(pdata,n[0], r[0], n[1]+1, r[1]-1, n[2]+1, r[2]-1,0,0),
										 lattice(pdata,n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2]+1, r[2]-1, n[3], r[3]),
										 w[0]),
									w[1]),
							   w[2]),
						  LERP(LERP(LERP(lattice(pdata,n[0], r[0], n[1], r[1], n[2], r[2], n[3]+1, r[3]-1),
										 lattice(pdata,n[0]+1, r[0]-1, n[1], r[1], n[2], r[2], n[3]+1, r[3]-1),
										 w[0]),
									LERP(lattice(pdata,n[0], r[0], n[1]+1, r[1]-1, n[2], r[2], n[3]+1, r[3]-1),
										 lattice(pdata,n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2], r[2], n[3]+1, r[3]-1),
										 w[0]),
									w[1]),
									LERP(LERP(lattice(pdata,n[0], r[0], n[1], r[1], n[2]+1, r[2]-1, n[3]+1, r[3]-1),
										 lattice(pdata,n[0]+1, r[0]-1, n[1], r[1], n[2]+1, r[2]-1, n[3]+1, r[3]-1),
										 w[0]),
									LERP(lattice(pdata,n[0], r[0], n[1]+1, r[1]-1, n[2]+1, r[2]-1,0,0),
										 lattice(pdata,n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2]+1, r[2]-1, n[3]+1, r[3]-1),
										 w[0]),
									w[1]),
							   w[2]),
						  w[3]);
			break;
	}
	return CLAMP(-0.99999f, 0.99999f, value);
}

#if 0
static float TCOD_noise_fbm( TCOD_noise_t noise,  float *f, float octaves )
{
	float tf[TCOD_NOISE_MAX_DIMENSIONS];
	perlin_data_t *pdata=(perlin_data_t *)noise;
	/* Initialize locals */
	float value = 0;
	int i,j;
	memcpy(tf,f,sizeof(float)*pdata->ndim);

	/* Inner loop of spectral construction, where the fractal is built */
	for(i=0; i<(int)octaves; i++)
	{
		value += TCOD_noise_get(noise,tf) * pdata->exponent[i];
		for (j=0; j < pdata->ndim; j++) tf[j] *= pdata->lacunarity;
	}

	/* Take care of remainder in octaves */
	octaves -= (int)octaves;
	if(octaves > DELTA)
		value += octaves * TCOD_noise_get(noise,tf) * pdata->exponent[i];
	return CLAMP(-0.99999f, 0.99999f, value);
}
#endif

static float TCOD_noise_turbulence( TCOD_noise_t noise, float *f, float octaves )
{
	float tf[TCOD_NOISE_MAX_DIMENSIONS];
	perlin_data_t *pdata=(perlin_data_t *)noise;
	/* Initialize locals */
	float value = 0;
	int i,j;
	memcpy(tf,f,sizeof(float)*pdata->ndim);

	/* Inner loop of spectral construction, where the fractal is built */
	for(i=0; i<(int)octaves; i++)
	{
		value += ABS(TCOD_noise_get(noise,tf)) * pdata->exponent[i];
		for (j=0; j < pdata->ndim; j++) tf[j] *= pdata->lacunarity;
	}

	/* Take care of remainder in octaves */
	octaves -= (int)octaves;
	if(octaves > DELTA)
		value += octaves * ABS(TCOD_noise_get(noise,tf)) * pdata->exponent[i];
	return CLAMP(-0.99999f, 0.99999f, value);
}

void TCOD_noise_delete(TCOD_noise_t noise) {
	free((perlin_data_t *)noise);
}


/*
 * Generates a 3d nebulae map of dimensions w,h,n with ruggedness rug
 */
static float* genNebulaeMap( const int w, const int h, const int n, float rug )
{
   int x, y, z;
   float f[3];
   float octaves;
   float hurst;
   float lacunarity;
   TCOD_noise_t noise;
   float *nebulae;
   float value;

   octaves = 3.;
   hurst = TCOD_NOISE_DEFAULT_HURST;
   lacunarity = TCOD_NOISE_DEFAULT_LACUNARITY;

   noise = TCOD_noise_new( 3, hurst, lacunarity );

   nebulae = malloc(sizeof(float)*w*h*n);
   if (nebulae == NULL) {
      WARN("Out of memory!");
      return NULL;
   }


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
   }

   TCOD_noise_delete( noise );

   return nebulae;
}


/*
 * Generates a SDL_Surface from a 2d nebulae map
 */
SDL_Surface* surfaceFromNebulaeMap( float* map, const int w, const int h )
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


/*
 * Generates nebulae and saves them for later usage
 */
void noise_generateNebulae( const int w, const int h )
{
   int i;
   float *nebu;
   SDL_Surface *sur;
   char nebu_file[PATH_MAX];

   /* Generate all the nebulae */
   nebu = genNebulaeMap( w, h, NEBULAE_Z, 15. );

   /* Save each nebulae as an image */
   for (i=0; i<NEBULAE_Z; i++) {
      sur = surfaceFromNebulaeMap( &nebu[ i*w*h ], w, h );
      snprintf( nebu_file, PATH_MAX, "%s/nebu_%02d.png", nfile_basePath(), i );
      SDL_SavePNG( sur, nebu_file );
      SDL_FreeSurface(sur);
   }

   /* Cleanup */
   free(nebu);
}


glTexture* noise_genCloud( const int w, const int h, double rug )
{
   float *map;
   SDL_Surface *sur;
   glTexture *tex;

   /*noise_generateNebulae(w,h);*/
   
   map = genNebulaeMap( w, h, 1, rug );
   sur = surfaceFromNebulaeMap( map, w, h );
   free(map);
   
   tex = gl_loadImage( sur );
   return tex;
}     


