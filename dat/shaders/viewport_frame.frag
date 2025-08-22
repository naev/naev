#include "lib/sdf.glsl"

uniform vec4 colour;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   colour_out   = colour;
   colour_out.a *= smoothstep(0.5, 0.8, abs(pos.x));
   colour_out.a *= smoothstep(-1.0, 0.0, -abs(pos.y));
}
