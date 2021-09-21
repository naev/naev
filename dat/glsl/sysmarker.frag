#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;
uniform int parami;

in vec2 pos;
out vec4 color_out;

void main(void) {
   const vec2 b1 = vec2( 0.9, 0.45 );
   const vec2 b2 = vec2( 0.9, 0.30 );
   float m = 1.0 / dimensions.x;

   color_out = color;
   vec2 b = (parami==1) ? b2 : b1;
   float d = sdEgg( pos, b );
   color_out.a *= smoothstep( -m, 0.0, -d - 0.75*b.y ) + smoothstep( -m, 0.0, -d ) * smoothstep( 0.0, m, d + 0.5*b.y );
}

