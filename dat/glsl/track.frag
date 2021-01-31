uniform vec4 c1;
uniform vec4 c2;
uniform float t1;
uniform float t2;
in vec2 pos;
out vec4 color_out;

void main(void) {
   float t;

   color_out = pos.x*(c2-c1) + c1;
   color_out.a *= 1. - clamp( pos.y*pos.y, 0., 1.);
   t = pos.x*(t2-t1) + t1;
   color_out.a *= t;

#include "colorblind.glsl"
}
