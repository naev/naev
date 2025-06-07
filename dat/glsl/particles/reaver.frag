#include "lib/sdf.glsl"
#include "lib/simplex.glsl"

uniform float u_r       = 0.0;
uniform float u_time    = 0.0;
uniform float u_fade    = 1.0;

in vec2 pos;
out vec4 colour_out;

const vec4 COLOUR = vec4( 0.95, 0.3, 0.5, 1.0 );
const vec4 COLOUR_FADE = vec4( 0.75, 0.75, 0.1, 1.0 );

vec4 blend( vec4 b, vec4 a )
{
   vec4 result;
   result.a   = a.a + b.a * (1.0-a.a);
   result.rgb = (a.rgb * a.a + b.rgb * b.a * (1.0-a.a)) / (result.a + 1e-6);
   return result;
}
void main (void)
{
   vec2 uv = pos;

   float fade = min(u_time*6.0,u_fade);
   float n1 = snoise( uv+vec2(2.0*u_time,u_r) );
   float n2 = snoise( uv+vec2(2.0*u_time,u_r+100.0) );
   float spin = u_time * (2.5 + 1.5*sin(2.0/7.0*u_time + u_r)) + 7.0*u_r;
   float st = sin(spin);
   vec2 offset1 = vec2( -0.35, 0.0 ) * st;
   vec2 offset2 = vec2(  0.35, 0.0 ) * st;

   float d1 = sdVesica( uv.yx+vec2(0.0,0.15*n1)+offset1, 2.3, 2.1 );
   float d2 = sdVesica( uv.yx+vec2(0.0,0.15*n2)+offset2, 2.3, 2.1 );

   vec4 col1 = mix( COLOUR_FADE, COLOUR, u_fade );
   col1.rgb += pow( smoothstep( 0.0, 0.1, -d1-0.1 ), 2.0 ) + vec3(0.1)*n1 - 0.1;
   col1.a *= smoothstep( 0.0, 0.2, -d1 );

   vec4 col2 = mix( COLOUR_FADE, COLOUR, u_fade );
   col2.rgb += pow( smoothstep( 0.0, 0.1, -d2-0.1 ), 2.0 ) + vec3(0.1)*n2 - 0.1;
   col2.a *= smoothstep( 0.0, 0.2, -d2 );

   if (cos(spin) < 0.0)
      colour_out = blend( col1, col2 );
   else
      colour_out = blend( col2, col1 );
}
