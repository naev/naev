#include "lib/math.glsl"
#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;
uniform float dt;

in vec2 pos;
out vec4 color_out;

void main(void) {
   float m = 1.0 / dimensions.x;
   vec2 uv = abs(pos);

   float d = sdArc( uv,
         vec2(sin(M_PI*0.75),cos(M_PI*0.75)),
         vec2(sin(M_PI/10.0),cos(M_PI/10.0)),
         1.0, 0.03 );

   d = min( d, sdUnevenCapsule( uv, vec2(M_SQRT1_2), vec2(0.8), 0.07, 0.02) );
   d -= (1.0+sin(3.0*dt)) * 0.007;
   d = max( -sdCircle( uv-vec2(M_SQRT1_2), 0.04 ), d );

   color_out = color;
   color_out.a *= 0.6*smoothstep( -m, m, -d );
}

