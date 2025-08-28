#include "lib/math.glsl"
#include "lib/sdf.glsl"

uniform vec4 colour;
uniform vec2 dimensions;
uniform float dt;
uniform float paramf;

in vec2 pos;
out vec4 colour_out;

#define CS(A)  vec2(sin(A),cos(A))
void main(void) {
   vec2 uv = pos;
   float m = 1.0 / dimensions.x;

   /* Outter stuff. */
   float w = 1.0 * m;
   float inner = 1.0-w-m;
   float d = sdArc( uv, CS(-M_PI/4.0), CS(M_PI/22.0*32.0), inner, w );

   /* Rotation matrix. */
   float dts = 0.1 * max( 0.5, 100.0 / paramf );
   float c, s;
   s = sin(dt*dts);
   c = cos(dt*dts);
   mat2 R = mat2( c, s, -s, c );

   vec2 auv = abs(uv);
   if (auv.y < auv.x)
      auv.xy = vec2( auv.y, auv.x );
   if (paramf > 100.0) {
      const float arcseg = M_PI/64.0;
      const vec2 shortarc = CS(arcseg);
      for (int i=2; i<16; i+=4)
         d = min( d, sdArc( auv, CS(M_PI/2.0+float(i)*arcseg),  shortarc, inner, w ) );

      /* Moving inner stuff. */
      uv = uv*R;
      const vec2 arclen = CS(M_PI/9.0);
      w = 2.0 * m;
      inner -= 2.0*(w+m);
      for (int i=0; i<5; i++)
         d = min( d, sdArc( uv, CS( float(i)*M_PI*2.0/5.0), arclen, inner, w ) );
   }
   else {
      const float arcseg = M_PI/32.0;
      const vec2 shortarc = CS(arcseg);
      for (int i=2; i<8; i+=4)
         d = min( d, sdArc( auv, CS(M_PI/2.0+float(i)*arcseg),  shortarc, inner, w ) );

      /* Moving inner stuff. */
      uv = uv*R;
      const vec2 arclen = CS(M_PI/6.0);
      w = 2.0 * m;
      inner -= 2.0*(w+m);
      for (int i=0; i<3; i++)
         d = min( d, sdArc( uv, CS( float(i)*M_PI*2.0/3.0), arclen, inner, w ) );
   }

   colour_out = colour;
   colour_out.a *= 0.6*smoothstep( -m, 0.0, -d );
}
