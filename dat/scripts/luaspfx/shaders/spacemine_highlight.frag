#include "lib/math.glsl"

const float PERIOD   = M_PI;
uniform float u_time = 0.0;

vec4 effect( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   float progress = mod( u_time, PERIOD ) / PERIOD;
   vec2 uv = texture_coords*2.0-1.0;
   float r = progress;
   float l = length(uv);
   if (l > r) /* TODO anti-aliasing instead of hard discard. */
      discard;
   float d = abs(length(uv)-r);
   colour.a *= smoothstep( -0.1, 0.0, -d );
   colour.a *= min( 1.0,  10.0*(1.0-progress) );
   return colour;
}
