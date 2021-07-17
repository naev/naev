// Libraries
#include "lib/math.glsl"
#include "lib/simplex.glsl"
#include "lib/cellular.glsl"
#include "lib/perlin.glsl"

uniform vec4 color;
uniform vec2 dimensions;
uniform float dt;

in vec2 pos;
out vec4 color_out;

void main(void) {
   vec2 pos_tex, pos_px;
   pos_tex.x = pos.x;
   pos_tex.y = 2.0 * pos.y - 1.0;
   pos_px = pos * dimensions;

   color_out = color;
}

