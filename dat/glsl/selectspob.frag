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

   const float dts = 0.5;
   float c, s;
   s = sin(dt*dts);
   c = cos(dt*dts);
   mat2 R = mat2( c, s, -s, c );

   vec2 uv     = R*pos;
   float w     = 0.5 * m;
   float inner = 1.0-w-2.0*m;
   const vec2 arclen = CS(M_PI/5.0);
   float d =   sdArc( uv, CS( 0.0*M_PI*2.0/3.0), arclen, inner, w );
   d = min( d, sdArc( uv, CS( 1.0*M_PI*2.0/3.0), arclen, inner, w ) );
   d = min( d, sdArc( uv, CS( 2.0*M_PI*2.0/3.0), arclen, inner, w ) );

   float alpha = smoothstep(     -m, 0.0, -d );
   float beta  = smoothstep( -2.0*m,  -m, -d );
   colour_out   = colour * vec4( vec3(alpha), beta );
}
