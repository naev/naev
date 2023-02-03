#include "lib/cellular.glsl"

uniform float time;
uniform vec2 globalpos;
uniform float alpha;
in vec2 localpos;
out vec4 colour_out;

const int ITERATIONS    = 3;
const float SCALAR      = 2.0;
const float SCALE       = 3.0;
const float TIME_SCALE  = 150.0;
const float smoothness  = 0.9;

void main (void)
{
   vec3 uv;

   /* Fallout */
   float dist = length(localpos);
   dist = (dist < 1.0-smoothness) ? 1.0 : (1.0 - dist) / smoothness;
   float a = smoothstep( 0.0, 1.0, dist );
   if (a <= 0.0) {
      discard;
      return;
   }

   /* Calculate coordinates */
   uv.xy = (localpos + globalpos) * 2.0;
   uv.z  = time * 0.1;

   /* Create the noise */
   float f;
   f  = (1.0-cellular2x2x2( uv     ).x) * 0.625;
   f += (1.0-cellular2x2x2( uv*2.0 ).x) * 0.375;

   const vec4 colour = vec4( 200.0/255.0, 32.0/255.0, 130.0/255.0, 1.0 );
   colour_out =  mix( vec4(0.0), colour, f );
   colour_out *= a * alpha;
}
