uniform vec4 color;
uniform float radius;
out vec4 color_out;
in vec2 pos;

void main(void) {
   color_out = color;
   color_out.a = 1. - abs(radius - length(pos) - 1.);

#include "colorblind.glsl"
}
