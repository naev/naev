#include "lib/sdf.glsl"
#include "lib/simplex.glsl"

uniform float u_time = 0.0;
uniform float u_speed = 1.0;
uniform float u_grain = 1.0;
uniform float u_r = 0.0;

vec4 effect( vec4 color, sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
   float progress = u_time * u_speed;
   vec2 uv = texture_coords*2.0-1.0;
   vec4 colour = vec4( 1.0, 0.8, 0.0, 1.0 );

   float d = sdCircle( uv, 0.8*progress );
   d = max( d, -sdCircle( uv, 2.8*(progress-0.5) ) );
   vec2 nuv = 3.0 * uv + vec2(u_r);
   float n = 0.3*snoise( u_grain * nuv );
   colour.a *= smoothstep( -0.2, 0.2, -d );

   colour   += 0.6 * n * smoothstep( -0.1, 0.1, -d );

   return colour;
}

#ifndef _LOVE
in vec2 pos;
out vec4 color_out;
uniform sampler2D dummy;
void main (void)
{
   color_out = effect( vec4(0.0), dummy, pos, vec2(0.0) );
}
#endif
