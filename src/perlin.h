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
float noise_get2( perlin_data_t* pdata, float f[2] );
float noise_get1( perlin_data_t* pdata, float f[1] );
/* Turbulence */
float noise_turbulence2( perlin_data_t* pdata, float f[2], int octaves );
/* Simplex noise. */
float noise_simplex1( perlin_data_t* noise, float f[1] );

/* NOTE: There are additional noise generators (turbulence1, turbulence3) in prior git revisions. */


/* High level. */
float* noise_genRadarInt( const int w, const int h, float rug );
float* noise_genNebulaPuffMap( const int w, const int h, float rug );


#endif
