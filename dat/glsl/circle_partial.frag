#include "lib/math.glsl"

uniform vec4 color;
uniform float radius;
uniform float angle1;
uniform float angle2;
out vec4 color_out;
in vec2 pos;

void main(void) {
   color_out = color;
   color_out.a *= clamp( radius - length(pos), 0.0, 1.0 );
   /* Try to cut. */
   float angle = mod(atan( pos.y, pos.x )-angle1+M_PI*2.0, M_PI*2.0);
   color_out.a *= step( 0.0, angle );
   color_out.a *= step( angle, angle2 );
}
