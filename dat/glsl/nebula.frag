uniform vec4 color;
uniform vec2 center;
uniform float radius;
out vec4 color_out;

void main(void) {
   color_out = color;
   float dist = distance(gl_FragCoord.xy, center);
   if (dist < radius) {
      color_out.a *= (dist / radius);
   }

#include "colorblind.glsl"
}
