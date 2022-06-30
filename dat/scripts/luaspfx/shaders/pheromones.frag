#include "lib/simplex.glsl"

const float LENGTH   = 8.0;
uniform float u_time = 0.0;

vec4 effect( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   float progress = fract( u_time / LENGTH );
   vec2 uv = texture_coords*2.0-1.0;
   float r = (progress*2.0-0.5);
   float d = abs(length(uv)-r);
   colour.a *= smoothstep( -0.5, 0.0, -d );
   colour.a -= length(uv);

   /* Discard no colour to go faster. */
   if (colour.a <= 0.0)
      discard;

   /* Add effect. */
   vec3 c = vec3( uv * 10.0, u_time * 0.3 );
   float n = snoise( c );
   n += (snoise( c * 3.0 ) * 0.5 + 0.2) / 3.0;
   colour.a *= 0.6+0.4 * n;

   return colour;
}
