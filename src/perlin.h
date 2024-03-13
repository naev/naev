#pragma once

struct perlin_data_s;
typedef struct perlin_data_s perlin_data_t;

/* Creation. */
perlin_data_t *noise_new( void );
void           noise_delete( perlin_data_t *pdata );

/* Simplex noise. */
float noise_simplex1( perlin_data_t *noise, float f[1] );

/* NOTE: There are additional noise generators (turbulence1, turbulence2,
 * turbulence3) in prior git revisions. */
