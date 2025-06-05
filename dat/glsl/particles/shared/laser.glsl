#include "lib/sdf.glsl"
#include "lib/simplex.glsl"

uniform float u_r       = 0.0;
uniform float u_time    = 0.0;
uniform float u_fade    = 1.0;

const vec4 COLOUR_FADE = vec4( 0.75, 0.75, 0.1, 1.0 );

in vec2 pos;
out vec4 colour_out;

void main (void)
{
   vec2 uv = pos;
   float fade = min(u_time*6.0,u_fade);
   float n = snoise( 1.5*uv+vec2(2.0*u_time,u_r) );

   float d = sdVesica( uv.yx+vec2(0.0,0.1*n), 2.3, 2.05+max(0.25*(1.0-fade),0.0) );

   colour_out = mix( COLOUR_FADE, COLOUR, u_fade );
   colour_out.rgb += pow( smoothstep( 0.0, 0.1, -d-0.15 ), 2.0 ) + vec3(0.1)*n - 0.1;
   colour_out.a *= smoothstep( 0.0, 0.2, -d );
}
