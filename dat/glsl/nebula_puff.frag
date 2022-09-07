#include "lib/perlin.glsl"

const float SCALAR = pow(2., 4./3.);

uniform vec3 nebu_col;
uniform float time;
uniform vec2 r;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   float a = 1.0-length( pos );
   a = smoothstep( 0.0, 1.0, a );
   if (a<0.0) {
      discard;
      return;
   }

   vec3 uv = vec3( pos+r, time );
   float f, scale;

   /* Create the noise */
   f  = abs( cnoise( uv ) );
   scale = SCALAR;
   f += abs( cnoise( uv * scale ) ) / scale;
   scale *= SCALAR;
   f += abs( cnoise( uv * scale ) ) / scale;

   colour_out = vec4( nebu_col, a*f );
}
