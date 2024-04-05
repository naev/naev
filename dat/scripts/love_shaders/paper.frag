precision highp float;

#include "lib/simplex.glsl"

const float u_r = %f;
const float u_sharp = %f;

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec4 texcolour = colour * texture( tex, uv );

   float n = 0.0;
   for (float i=1.0; i<8.0; i=i+1.0) {
      float m = pow( 2.0, i );
      n += snoise( px * u_sharp * 0.003 * m + 1000.0 * u_r ) * (1.0 / m);
   }

   texcolour.rgb *= 0.68 + 0.3 * n;

   return texcolour;
}
