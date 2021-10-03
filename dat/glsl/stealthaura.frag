#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;

in vec2 pos;
out vec4 color_out;

void main(void) {
   float m     = 1.0 / dimensions.x;
   float d     = sdCircle( pos, 1.0-m );
   float alpha = smoothstep(-m, 0.0, -d);
   color_out   = color;
   color_out.a *= alpha;
   color_out.a *= length(pos);
}
