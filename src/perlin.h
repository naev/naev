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


#ifndef _TCOD_PERLIN_H
#define _TCOD_PERLIN_H


#define NOISE_MAX_OCTAVES            4 /**< Default octaves for noise. */
#define NOISE_DEFAULT_HURST          0.5 /**< Default hurst for noise. */
#define NOISE_DEFAULT_LACUNARITY     2. /**< Default lacunarity for noise. */


struct perlin_data_s;
typedef struct perlin_data_s perlin_data_t;


/* Creation. */
perlin_data_t* noise_new( int dim, float hurst, float lacunarity );
void noise_delete( perlin_data_t* pdata );

/* Basic perlin noise */
float noise_get3( perlin_data_t* pdata, float f[3] );
float noise_get2( perlin_data_t* pdata, float f[2] );
float noise_get1( perlin_data_t* pdata, float f[1] );
/* Turbulence */
float noise_turbulence3( perlin_data_t* pdata, float f[3], int octaves );
float noise_turbulence2( perlin_data_t* pdata, float f[2], int octaves );
float noise_turbulence1( perlin_data_t* pdata, float f[1], int octaves );
/* Simplex noise. */
float noise_simplex1( perlin_data_t* noise, float f[1] );


/* High level. */
float* noise_genRadarInt( const int w, const int h, float rug );
float* noise_genNebulaMap( const int w, const int h, const int n, float rug );
float* noise_genNebulaPuffMap( const int w, const int h, float rug );


#endif
