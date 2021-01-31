#include "math.h"

uniform vec4 c1;
uniform vec4 c2;
uniform float t1;
uniform float t2;
in vec2 pos;
out vec4 color_out;

void main(void) {
   float t, d;

   // Interpolating base color
   color_out = pos.x*(c2-c1) + c1;

   // Modulate alpha based on dispersion
   d = pos.y;
   //d = 1. - min( abs(pos.y), 1. );
   d = pow( cos( M_PI * d / 2. ), 3. );
   color_out.a *= d;

   // Modulate alpha base on length
   t = pos.x*(t2-t1) + t1; // interpolate between both points
   t = 1. - pow( max(0.0, abs(1.-t) * 2.0 - 1.0 ), 1.0 );
   color_out.a *= t;

#include "colorblind.glsl"
}
