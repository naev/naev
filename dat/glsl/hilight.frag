#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;
uniform float dt;

in vec2 pos;
out vec4 color_out;

void main(void) {
   float a     = 1.0 - 0.5*fract( dt/2.0 );
   float d     = sdCircle( pos*dimensions, a*dimensions.x-1.0 );
   float alpha = smoothstep(-1.0, 0.0, -d);
   color_out   = color;
   color_out.a *= alpha;
}

