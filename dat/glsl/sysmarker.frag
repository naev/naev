#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;

in vec2 pos;
out vec4 color_out;

void main(void) {
   const vec2 b = vec2( 0.9, 0.6 );
   float m = 1.0 / dimensions.x;

   color_out = color;
   float dist = sdRhombus( pos, b );
   dist = max( -sdRhombus( pos*2.0, b ), dist );
   dist = min(  dist, sdRhombus( pos*4.0, b ) );
   color_out.a *= smoothstep( -m, m, -dist );
}

