uniform float a;
out vec4 color_out;

void main(void) {
   color_out = vec4( 1, 1, 1, a );

#include "colorblind.glsl"
}
