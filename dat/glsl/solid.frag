uniform vec4 color;
out vec4 color_out;

void main(void) {
   color_out = color;

#include "colorblind.glsl"
}
