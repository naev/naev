#include "lib/flow2D.glsl"
#include "lib/perlin3D.glsl"

const int ITERATIONS = 3;
const float SCALAR = pow(2., 4./3.);

uniform vec4 color;
uniform vec2 center;
uniform float radius;
uniform float time;
out vec4 color_out;

void main(void) {
   float f = 0.0;
   vec3 uv;

   // Calculate coordinates
   uv.xy = 4. * (gl_FragCoord.xy-center) / radius;
   uv.z = time;

   // Create the noise
   for (int i=0; i<ITERATIONS; i++) {
      float scale = pow(SCALAR, i);
      //f += abs( srnoise( uv*scale, time/scale ) ) / scale;
      f += abs( cnoise( uv * scale ) ) / scale;
   }
   color_out = mix( vec4( 0, 0, 0, 1 ), color, .1 + f );

#include "colorblind.glsl"
}
