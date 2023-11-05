// NOTE do not use directly, set the value of COLOUR and include this file
#include "lib/sdf.glsl"
#include "lib/simplex.glsl"

//uniform vec3 dimensions;
uniform float u_r       = 0.0;
uniform float u_time    = 0.0;
uniform float u_fade    = 1.0;

in vec2 pos;
out vec4 colour_out;

//const vec4 COLOUR = vec4( 0.6, 0.6, 0.9, 1.0 );

void main (void)
{
   vec2 uv = pos;
   colour_out = COLOUR;

   float d = sdCircle( uv, 1.0 );

   colour_out.a *= smoothstep( 0.0, 0.5, -d );
   if (colour_out.a <= 0.0)
      discard;

   vec2 uvoff = vec2(1.4*u_time,u_r);
   vec2 nuv = uv+uvoff;
   nuv *= 1.5;
   float n = snoise( nuv );
   n += 0.5*snoise( 2.0*nuv );

   colour_out.a *= mix( 0.5 + 0.5*snoise( normalize(uv)*0.7+uvoff ), 1.0, colour_out.a );

   colour_out.rgb += smoothstep( 0.3, 1.0, -d ) * (n*0.7+0.3);
}
