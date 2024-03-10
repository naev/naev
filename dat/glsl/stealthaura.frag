#include "lib/sdf.glsl"

uniform vec4 colour;
uniform vec2 dimensions;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   float m     = 1.0 / dimensions.x;
   float d     = sdCircle( pos, 1.0-m );
   float alpha = smoothstep(-m, 0.0, -d);
   colour_out   = colour;
   colour_out  *= alpha * length(pos);
}
