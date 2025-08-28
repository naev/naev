#include "lib/perlin.glsl"

const float SCALAR = pow(2., 4./3.);

layout(std140) uniform PuffData {
   vec2 screen;
   vec3 offset;
   vec3 colour;
   float elapsed;
   float scale;
};

in vec2 fragpos;
in vec2 rand;
layout(location = 0) out vec4 colour_out;

void main(void) {
   float a = 1.0-length( fragpos );
   a = smoothstep( 0.0, 1.0, a );
   if (a<0.0) {
      discard;
      return;
   }

   vec3 uv = vec3( fragpos+rand, elapsed );
   float f, scale;

   /* Create the noise */
   f  = abs( cnoise( uv ) );
   scale = SCALAR;
   f += abs( cnoise( uv * scale ) ) / scale;
   scale *= SCALAR;
   f += abs( cnoise( uv * scale ) ) / scale;

   colour_out = vec4( colour, a*f );
}
