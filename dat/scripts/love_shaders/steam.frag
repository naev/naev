#include "lib/math.glsl"
#include "lib/simplex.glsl"

uniform float u_time;

const float strength = %f;
const float speed    = %f;
const float u_r      = %f;

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec4 texcolour = colour * texture( tex, uv );

   vec2 offset = vec2( 50.0*sin( M_PI*u_time * 0.001 * speed ), -0.3*u_time*speed );

   float n = 0.0;
   for (float i=1.0; i<4.0; i=i+1.0) {
      float m = pow( 2.0, i );
      n += snoise( offset +  px * strength * 0.0015 * m + 1000.0 * u_r ) * (1.0 / m);
   }

   texcolour.a *= 0.68 + 0.3 * n;

   return colour * texcolour;
}
