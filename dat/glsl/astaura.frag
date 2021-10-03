#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;

in vec2 pos;
out vec4 color_out;

void main(void) {
   float d = sdCircle( pos*dimensions, dimensions.x-1.0 );
   float alpha = smoothstep(-1.0, 0.0, -d);
   color_out = color * alpha;
}

