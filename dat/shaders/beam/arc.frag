#include "beam/common.glsl"
#include "lib/simplex.glsl"

void main (void) {
   float m, y, v;
   vec2 ncoord;

   // Modulate alpha base on length
   colour_out.a = colour.a * beamfade( pos_px.x, pos_tex.x );

   // Modulate alpha based on dispersion
   m = 4.0;

   // Modulate width
   ncoord = vec2( 0.03 * pos_px.x, 7.0*ANIM_SPEED*dt ) + 1000.0 * r;
   y = clamp( pos_tex.y + 0.7 * snoise( ncoord ), -1.0, 1.0 );
   v = sharpbeam( y, m );
   y = clamp( pos_tex.y + 0.7 * snoise( 1.5*ncoord ), -1.0, 1.0 );
   v += sharpbeam( y, 2.0*m );
   y = clamp( pos_tex.y + 0.7 * snoise( 2.0*ncoord ), -1.0, 1.0 );
   v += sharpbeam( y, 4.0*m );

   v = abs(v);
   colour_out.rgb  = mix( colour.rgb, vec3(1.0), v*0.5 );
   colour_out.rgb  = pow( colour_out.rgb, vec3(3.0) );
   colour_out.a   *= min(1.0, v);
}
