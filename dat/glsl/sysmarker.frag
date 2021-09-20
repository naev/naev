#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;
uniform int parami;

in vec2 pos;
out vec4 color_out;

void main(void) {
   const vec2 b1 = vec2( 0.9, 0.6 );
   const vec2 b2 = vec2( 0.9, 0.4 );
   float m = 1.0 / dimensions.x;

   color_out = color;
   float d;
   if (parami==1) {
      const vec2 b = b2;
      d = sdRhombus( pos, b );
      d = max( -sdRhombus( pos*2.0, b ), d );
      d = min(  d, sdRhombus( pos*4.0, b ) );
   }
   else {
      const vec2 b = b1;
      d = sdRhombus( pos, b );
      d = max( -sdRhombus( pos*2.0, b ), d );
      d = min(  d, sdRhombus( pos*4.0, b ) );
   }
   color_out.a *= smoothstep( -m, 0.0, -d );
}

