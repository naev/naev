#include "lib/math.glsl"
#include "lib/sdf.glsl"

uniform vec4 colour;
uniform vec2 dimensions;
uniform float dt;

in vec2 pos;
out vec4 colour_out;

#define CS(A)  vec2(sin(A),cos(A))
void main(void) {
   float m = 1.0 / dimensions.x;
   vec2 uv = abs(pos);

   float d = sdArc( uv, CS(M_PI*0.75), CS(M_PI/10.0), 1.0, 0.02 );

   d = min( d, sdUnevenCapsule( uv, vec2(M_SQRT1_2), vec2(0.8), 0.07, 0.02) );
   d -= (1.0+sin(3.0*dt)) * 0.007;
   d = max( -sdCircle( uv-vec2(M_SQRT1_2), 0.04 ), d );

   colour_out = colour;
   colour_out.a *= 0.6*smoothstep( -m, 0.0, -d );
}
