#include "lib/sdf.glsl"
#include "lib/simplex.glsl"

uniform float u_r       = 0.0;
uniform float u_time    = 0.0;
uniform float u_fade    = 1.0;

in vec2 pos;
out vec4 colour_out;

const vec3 COLOUR_IN = vec3( 1.0, 0.1, 0.4 );
const vec3 COLOUR_OUT = vec3( 1.5, 0.8, 1.5 );
const vec3 COLOUR_START = vec3( 1.0, 1.0, 0.1 );

void main (void)
{
   vec2 uv = pos;

   vec2 r = vec2(-u_time*0.25, u_r*100.0);

   float n = snoise( 0.7*(uv + r) )*0.5+0.25;
   n += snoise( 2.0*(uv + r) )*0.25;

   colour_out.a = n - max( 0.0, pow(length(uv),4.0) ) - smoothstep( 1.0, 0.0, 5.0*u_time);
   colour_out.rgb = mix( COLOUR_OUT, COLOUR_IN, colour_out.a+0.5 );
   colour_out.rgb = mix( COLOUR_START, colour_out.rgb, u_time*1.7 );
   colour_out.a *= u_fade;
}
