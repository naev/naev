#include "lib/simplex.glsl"

uniform float time;
uniform vec2 globalpos;
uniform float alpha;
in vec2 localpos;
out vec4 colour_out;

const int ITERATIONS    = 4;
const float SCALAR      = 2.0;
const float SCALE       = 3.0;
const float TIME_SCALE  = 10.0;
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
   float f = 0.0;
   for (int i=0; i<ITERATIONS; i++) {
      float scale = pow(SCALAR, float(i));
      f += (snoise( uv * scale )*0.5 + 0.2) / scale;
   }

   const vec4 colour = vec4( 0.937, 0.102, 0.300, 0.8 );
   colour_out =  mix( vec4(0.0), colour, f );
   colour_out *= a * alpha;
}
