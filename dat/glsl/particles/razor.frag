// NOTE do not use directly, set the value of COLOUR and #include
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

   vec2 nuv = uv+vec2(2.0*u_time,u_r);
   float n = snoise( nuv );
   //n += 0.5*snoise( 2.0*nuv ); // At the sizes we render, probably don't need two octaves...

   float d = sdCircle( uv, 1.0 );

   colour_out.a *= smoothstep( 0.0, 0.5, -d );
   colour_out.rgb += smoothstep( 0.3, 1.0, -d ) * (n*0.7+0.3);
}
