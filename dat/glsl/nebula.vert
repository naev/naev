#include "lib/colour.glsl"

layout(std140) uniform NebulaData {
   float hue;
   float horizon;
   float eddy_scale;
   float elapsed;
   float nonuniformity;
   float volatility;
   float saturation;
   vec2 camera;
};

layout(location = 0) in vec4 vertex;
out vec4 base_col;

void main(void) {
   gl_Position = vertex;
   base_col = vec4( hsv2rgb( vec3(hue, saturation, 0.5*saturation + 0.5) ), 1.0 );
}
