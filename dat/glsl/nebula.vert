#include "lib/colour.glsl"

uniform float hue;
uniform mat4 projection;
uniform float saturation;
in vec4 vertex;
out vec4 base_col;

void main(void) {
   gl_Position = projection * vertex;
   base_col = vec4( hsv2rgb( vec3(hue, saturation, 1.0) ), 1.0 );
}
