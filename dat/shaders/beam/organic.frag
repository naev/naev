#include "beam/common.glsl"
#include "lib/simplex.glsl"
#include "lib/cellular.glsl"

void main (void) {
   float m, p;
   vec2 coords;
   const float range = 0.3;

   colour_out.a *= beamfade( pos_px.x, pos_tex.x );

   // Modulate alpha based on dispersion
   m = 3.0;

   coords = pos_px + vec2( -200.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   p = 1.0 - 0.7*cellular2x2( 0.13 * coords ).x;

   // Modulate width
   colour_out.a   *= p * smoothbeam( pos_tex.y, m );
   colour_out.rgb = mix( colour.rgb, vec3(1.0), max(0.0, 10.0*(p-0.9)) );
   colour_out.a    = pow( colour_out.a, 1.5 );

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   float v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   colour_out.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );
}
