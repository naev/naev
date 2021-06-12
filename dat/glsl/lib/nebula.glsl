#ifndef _NEBULA_GLSL
#  define _NEBULA_GLSL

#include "lib/perlin.glsl"
#include "lib/simplex.glsl"
#include "lib/colour.glsl"

const int ITERATIONS = 3;
const float SCALAR = pow(2., 4./3.);
const float COLOUR_DISPERSION = 0.02;

float nebula_hue( float hue, vec3 uv )
{
   hue += COLOUR_DISPERSION * snoise( 0.1 * uv );
   return fract(hue);
}

vec4 nebula( vec4 bg, vec2 rel_pos, float time, float hue, float value, float brightness )
{
   float f, scale;
   vec3 uv, rgb, hsv;

   /* Calculate coordinates */
   uv.xy = rel_pos;
   uv.z = time;

   /* Create the noise */
   scale = 1.0;
   f  = abs( cnoise( uv * scale ) ) / scale;
   scale *= SCALAR;
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
   hsv = vec3( hue, value, 1.0 );
   rgb = hsv2rgb( hsv );

   return mix( bg, vec4( rgb, 1.0 ), brightness+(1.0-brightness)*f );
}

#endif /* _NEBULA_GLSL */
