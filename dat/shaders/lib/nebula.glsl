#ifndef _NEBULA_GLSL
#  define _NEBULA_GLSL

#include "lib/perlin.glsl"
#include "lib/simplex.glsl"
#include "lib/colour.glsl"

const int ITERATIONS = 3;
const float SCALAR = pow(2.0, 4.0/3.0);
const float COLOUR_DISPERSION = 0.02;

float nebula_hue( float hue, vec3 uv )
{
   hue += COLOUR_DISPERSION * snoise( 0.1 * uv );
   return fract(hue);
}

vec4 nebula( vec4 bg, vec2 rel_pos, float time, float hue, float value, float volatility, float brightness )
{
   float f, scale;
   vec3 uv, rgb, hsv;

   /* Calculate coordinates */
   uv.xy = rel_pos;
   uv.z = time * 0.5;

   /* Create the noise */
   //scale = 1.0;
   f  = abs( cnoise( uv ) );
   scale = SCALAR;
   f += abs( cnoise( uv * scale ) ) / scale;
   scale *= SCALAR;
   f += abs( cnoise( uv * scale ) ) / scale;
   /* Rolled loop
   for (int i=0; i<ITERATIONS; i++) {
      scale = pow(SCALAR, i);
      f += abs( cnoise( uv * scale ) ) / scale;
   }
   */

   /* Choose the colour. */
   hue = nebula_hue( hue, uv );
   hsv = vec3( hue, value, 0.5 + 0.5*value );
   rgb = hsv2rgb( hsv );
   vec4 colour = mix( bg, vec4( rgb, 1.0 ), brightness+(1.0-brightness)*f );

   /* Some volatility */
   float v = clamp( (volatility-5.0) / 15.0, 0.0, 1.0 );
   if (v > 0.0) {
      float t = time*(0.5 + 3.0*v); // 2.0 is good
      float flash_0 = sin(t);
      float flash_1 = sin(15.0 * t);
      float flash_2 = sin(2.85 * t);
      float flash_3 = sin(5.18 * t);
      uv    *= vec3(0.1, 0.1, 5.0);
      float flash = max( 0.0, snoise(uv) * flash_0 * flash_1 * flash_2 * flash_3 );
      colour.rgb += vec3( 2.0*(f+0.5*flash)*flash*(0.5+0.5*flash_2) );
   }

   return colour;
}

#endif /* _NEBULA_GLSL */
