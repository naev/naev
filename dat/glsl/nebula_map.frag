#include "lib/perlin.glsl"

const int ITERATIONS = 3;
const float SCALAR = pow(2., 4./3.);

uniform vec4 color;
uniform mat4 projection;
uniform float eddy_scale;
uniform float time;
uniform vec2 globalpos;
in vec2 localpos;
out vec4 color_out;

const float smoothness = 0.5;

void main(void) {
   float f = 0.0;
   vec3 uv;

   // Calculate coordinates
   vec2 rel_pos = gl_FragCoord.xy + projection[3].xy - globalpos;
   uv.xy = rel_pos / eddy_scale;
   uv.z = time;

   // Create the noise
   for (int i=0; i<ITERATIONS; i++) {
      float scale = pow(SCALAR, i);
      f += abs( cnoise( uv * scale ) ) / scale;
   }

   color_out = mix( vec4( 0.0 ), color, 0.2 + f );

   // Fallout
   float dist = length(localpos);
   dist = (dist < 1.0-smoothness) ? 1.0 : (1.0 - dist) / smoothness;
   color_out.a *= smoothstep( 0.0, 1.0, dist );

#include "colorblind.glsl"
}
