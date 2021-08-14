#include "lib/math.glsl"
#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;
uniform float dt;

in vec2 pos;
out vec4 color_out;

#define CS(A)  vec2(sin(A),cos(A))
void main(void) {
   vec2 uv = pos;
   float m = 1.0 / dimensions.x;

	/* Outter stuff. */
	float w = 1.0 / dimensions.x;
	float inner = 1.0-w-m;
	float d = sdArc( uv, CS(-M_PI/4.0), CS(M_PI/22.0*32.0), inner, w );

   const float arcseg = M_PI/33.0;
   const vec2 shortarc = CS(arcseg);
   vec2 auv = abs(uv);
   d = min( d, sdArc( auv, CS(M_PI/2.0+2.0*arcseg), shortarc, inner, w ) );
   d = min( d, sdArc( auv, CS(M_PI/2.0+6.0*arcseg), shortarc, inner, w ) );
   d = min( d, sdArc( auv, CS(M_PI/2.0+10.0*arcseg), shortarc, inner, w ) );
   d = min( d, sdArc( auv, CS(M_PI/2.0+14.0*arcseg), shortarc, inner, w ) );

	/* Moving inner stuff. */
   const vec2 arclen = CS(M_PI/6.0);
	w = 2.0 / dimensions.x;
	float o = 0.1 * dt;
	inner -= 2.0*w+m;
	d = min( d, sdArc( uv, CS(o), arclen, inner, w ) );
	o += M_PI * 2.0/3.0;
	d = min( d, sdArc( uv, CS(o), arclen, inner, w ) );
	o += M_PI * 2.0/3.0;
	d = min( d, sdArc( uv, CS(o), arclen, inner, w ) );

   color_out = color;
   color_out.a *= 0.6*smoothstep( -m, m, -d );
}

