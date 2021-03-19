#ifndef _NEBULA_GLSL
#  define _NEBULA_GLSL

#include "lib/perlin.glsl"
#include "lib/simplex.glsl"
#include "lib/colour.glsl"

const int ITERATIONS = 3;
const float SCALAR = pow(2., 4./3.);
const float COLOUR_DISPERSION = 0.05;

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
   hue += COLOUR_DISPERSION * snoise( 0.1 * uv );
   hue = fract(hue);
   hsv = vec3( hue, value, 1.0 );
   rgb = hsv2rgb( hsv );

   return mix( bg, vec4( rgb, 1.0 ), brightness+(1.0-brightness)*f );
}

#endif /* _NEBULA_GLSL */
