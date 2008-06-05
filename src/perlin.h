#ifndef _TCOD_PERLIN_H
#define _TCOD_PERLIN_H


#include "opengl.h"

typedef void *TCOD_noise_t;

#define TCOD_NOISE_MAX_OCTAVES			128	
#define TCOD_NOISE_MAX_DIMENSIONS		4
#define TCOD_NOISE_DEFAULT_HURST        0.5f
#define TCOD_NOISE_DEFAULT_LACUNARITY   2.0f

TCOD_noise_t TCOD_noise_new(int dimensions, float hurst, float lacunarity );
/* basic perlin noise */
float TCOD_noise_get( TCOD_noise_t noise, float *f );
/* fractional brownian motion */
float TCOD_noise_fbm( TCOD_noise_t noise, float *f, float octaves );
/* turbulence */
float TCOD_noise_turbulence( TCOD_noise_t noise, float *f, float octaves );
void TCOD_noise_delete(TCOD_noise_t noise);

glTexture* noise_genCloud( const int w, const int h, double rug );

#endif
