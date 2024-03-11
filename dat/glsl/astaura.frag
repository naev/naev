#include "lib/sdf.glsl"

uniform vec4 colour;
uniform vec2 dimensions;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   float d = sdCircle( pos*dimensions, dimensions.x-1.0 );
   float alpha = smoothstep(-1.0, 0.0, -d);
   colour_out = colour * alpha;
}
