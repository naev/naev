#include "lib/sdf.glsl"

const float MARGIN   = 30.0; /**< How much margin to leave when fading in/out. */

uniform vec4 colour;
uniform vec2 dimensions;
//uniform float dt;
//uniform float r;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   vec2 uv = pos * dimensions;
   float d = sdBox( uv, dimensions-vec2(2.0) );

   /*
   float alpha = smoothstep(-1.0,  0.0, -d);
   float beta  = smoothstep(-2.0, -1.0, -d);
   colour_out   = colour * vec4( vec3(alpha), beta );
   */
   colour_out   = colour;
   colour_out.a *= smoothstep(-2.0, 0.0, -d); /* no outline, more blur */
   colour_out.a *= smoothstep(dimensions.x, dimensions.x-MARGIN, length(uv));
}
