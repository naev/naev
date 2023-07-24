#include "lib/sdf.glsl"
#include "lib/cellular.glsl"

//uniform vec3 dimensions;

uniform float u_r       = 0.0;
uniform float u_time    = 0.0;
uniform float u_fade    = 1.0;

in vec2 pos;
out vec4 colour_out;

const vec4 COLOUR= vec4( 0.2, 0.8, 0.4, 1.0 );

void main (void)
{
   vec2 uv = pos;
   colour_out = COLOUR;
   float t = min( 1.0, u_time*3.0 );

   float dout = sdCircle( uv, 0.98*t );
   float din  = sdCircle( uv, 0.95*t );

   vec3 uvt = 2.0*vec3( uv, length(uv)-0.4*u_time+10.0*u_r );
   float f = (1.0-cellular( uvt ).x);

   colour_out.a *= 1.0-smoothstep( -0.05, 1.0, -din );
   colour_out.a *= 1.0-smoothstep( 0.2, 0.0, -dout-0.08*f+0.1 );
   colour_out.a *= u_fade;
   colour_out.a *= 0.7+0.4*f;
}
