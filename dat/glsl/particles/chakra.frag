#include "lib/sdf.glsl"
#include "lib/simplex.glsl"

//uniform vec3 dimensions;

uniform float u_r;
uniform float u_time;
uniform float u_fade;

in vec2 pos;
out vec4 colour_out;

const vec4 COLOUR = vec4( 1.0, 0.8, 0.0, 1.0 );

void main (void)
{
   const vec2 b = vec2( 0.8, 0.5 );
   vec2 uv = pos;
   colour_out = COLOUR;

   float d = sdEgg( uv, b );
   vec2 nuv = vec2(2.0,4.0) * uv * vec2( exp(uv.x), pow(uv.y,0.5) );
   float n = 0.3*snoise( nuv + 3.0*vec2(u_time,u_r) );
   colour_out.a *= smoothstep( -0.1, 0.8, -d ) * (n+0.6);
   colour_out += smoothstep( -0.4, 0.7, -d );

   colour_out.a *= u_fade * min(10.0*u_time,1.0);
}
