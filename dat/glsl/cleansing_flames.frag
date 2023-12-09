#include "lib/sdf.glsl"
#include "lib/simplex.glsl"

uniform float u_time = 0.0;
uniform float u_speed = 1.0;
uniform float u_grain = 1.0;
uniform float u_r = 0.0;

vec4 effect( vec4 colour, sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
   float progress = u_time * u_speed;
   vec2 uv = texture_coords*2.0-1.0;
   vec4 colour = vec4( 1.0, 0.8, 0.0, 1.0 );

   float d = sdCircle( uv, 1.0*progress-0.1 );
   d = max( d, -sdCircle( uv, 1.5*progress-0.4 ) );
   vec2 nuv = 4.0 * u_grain * uv;
   float n = 0.675*snoise( nuv + vec2(u_r) );
   n += 0.325*snoise( nuv*2.0 - vec2(u_r) );
   colour.a *= smoothstep( -0.1, 0.1, -d ) * (n+0.8);

   colour.a   += 0.9 * smoothstep( -0.1, 0.1, -d );

   return colour;
}
