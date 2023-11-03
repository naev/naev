#include "lib/sdf.glsl"
#include "lib/simplex.glsl"

//uniform vec3 dimensions;

uniform float u_r       = 0.0;
uniform float u_time    = 0.0;
uniform float u_fade    = 1.0;

in vec2 pos;
out vec4 colour_out;

const vec4 COLOUR = vec4( 0.95, 0.1, 0.3, 1.0 );

void main (void)
{
   const vec2 b = vec2( 0.8, 0.5 );
   vec2 uv = pos;
   colour_out = COLOUR;

   float n = snoise( uv+vec2(6.0*u_time,u_r) );

   float d = sdRoundedCross( 1.25*uv+0.03*n, 0.25 );
   float din = sdVesica( uv.yx+0.05*n, 1.0, 0.8 );

   colour_out.rgb += pow( smoothstep( 0.0, 0.2, -din ), 2.0 );
   colour_out.a *= smoothstep( 0.0, 0.2, -d+0.25 );
   colour_out.a *= u_fade;
}
