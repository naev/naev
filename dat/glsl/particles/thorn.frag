#include "lib/sdf.glsl"
#define CELLULAR_NOISE_ACCURATE
#include "lib/cellular.glsl"

//uniform vec3 dimensions;

uniform float u_r       = 0.0;
uniform float u_time    = 0.0;
uniform float u_fade    = 1.0;

in vec2 pos;
out vec4 colour_out;

const vec3 COLOUR = GAMMA_TO_LINEAR_VEC3(0.45, 0.15, 0.15);
const vec3 COLOUR_FADE = GAMMA_TO_LINEAR_VEC3(0.9, 0.5, 0.05 );

void main (void)
{
   float fade = min(u_time*6.0,u_fade);
   vec2 c = cellular( vec3( 4.0*uv, 100.*u_r) );
   float n = (c.y-c.x);

   uv.y = -uv.y + 0.8;
   float d = sdUnevenCapsuleY( uv*0.8, 0.1, 0.4, 1.0 ) + n*0.2;
   d *= 3.0 * pow( smoothstep( 0.0, 0.3, -d ), 2.0 );

   colour_out.rgb = mix( COLOUR_FADE,
         COLOUR - d * (COLOUR_FADE-COLOUR),
         u_fade );
   colour_out.a = smoothstep( 0.0, 0.01, -d );
}
