// TODO move this math include to flow2D.glsl. For some reason it doesn't work...
#include "lib/math.glsl"
#include "lib/flow2D.glsl"

const int ITERATIONS = 3;
const float SCALAR = pow(2., 4./3.);

uniform vec4 color;
uniform vec2 center;
uniform float radius;
uniform float time;
out vec4 color_out;

void main(void) {
   float f = 0.0;
   vec2 uv = (gl_FragCoord.xy-center)*4./radius;
   for (int i=0; i<ITERATIONS; i++) {
      float scale = pow(SCALAR, i);
      f += abs( srnoise( uv*scale, .5*time/scale ) ) / scale;
   }
   color_out = mix( vec4( 0, 0, 0, 1 ), color, .1 + f );

#include "colorblind.glsl"
}
