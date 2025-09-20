uniform sampler2D MainTex;
in vec4           VaryingTexCoord;
out vec4          colour_out;

#include "lib/math.glsl"

#define A_GPU 1
#define A_GLSL 1
#include "lib/ffx_a.h"

#define FSR_RCAS_F 1
AU4 con0;

AF4 FsrRcasLoadF( ASU2 p )
{
   return AF4( texelFetch( MainTex, p, 0 ) );
}
void FsrRcasInputF( inout AF1 r, inout AF1 g, inout AF1 b )
{
}

#include "lib/ffx_fsr1.h"

// prng: A simple but effective pseudo-random number generator [0;1]
float prng( vec2 uv, float time )
{
   return random( uv + fract( time ) );
}

// pdf: [-0.5;0.5[
// Removes noise modulation effect by reshaping the uniform/rectangular noise
// distribution (RPDF) into a Triangular (TPDF) or Gaussian Probability Density
// Function (GPDF).
// shape = 1.0: Rectangular
// shape = 0.5: Triangular
// shape < 0.5: Gaussian (0.2~0.4)
float pdf( float noise, float shape )
{
   float orig = noise * 2.0 - 1.0;
   noise      = pow( abs( orig ), shape );
   noise *= sign( orig );
   noise -= sign( orig );
   return noise * 0.5;
}

const float FSR_GRAINCOLOR = 0.0;
const float FSR_

   void
   main()
{
   AF4 Gamma2Color = texture( sampler, tex_coord );

   // FSR - [LFGA] LINEAR FILM GRAIN APPLICATOR
   if ( FSR_GRAINCOLOR == 0.0 ) {
      float noise = pdf( prng( tex_coord, params.FrameCount * 0.11 ),
                           params.FSR_GRAINPDF );
      FsrLfgaF( Gamma2Color, vec3( noise ), params.FSR_FILMGRAIN );
   } else {
      vec3 rgbNoise = vec3(pdf( prng( tex_coord, params.FrameCount * 0.11 ),
                              params.FSR_GRAINPDF ),
                           pdf( prng( tex_coord, params.FrameCount * 0.13 ),
                              params.FSR_GRAINPDF ),
                           pdf( prng( tex_coord, params.FrameCount * 0.17 ),
                              params.FSR_GRAINPDF ) );
      FsrLfgaF( Gamma2Color, rgbNoise, params.FSR_FILMGRAIN );
   }

   colour_out = vec4( Gamma2Color, 1.0 );
}
