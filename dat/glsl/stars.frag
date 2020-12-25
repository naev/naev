in float brightness_out;
out vec4 color_out;

void main(void) {
   color_out = vec4(1., 1., 1., brightness_out);

#include "colorblind.glsl"
}
