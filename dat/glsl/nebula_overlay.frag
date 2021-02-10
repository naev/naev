uniform vec4 color;
uniform vec2 center;
uniform float radius;
out vec4 color_out;

void main(void) {
   color_out = color;
   float dist = distance(gl_FragCoord.xy, center);
   color_out.a *= smoothstep( 0, radius, dist );

#include "colorblind.glsl"
}
