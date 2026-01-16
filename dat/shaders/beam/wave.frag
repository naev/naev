#include "beam/common.glsl"
#include "lib/simplex.glsl"

void main (void) {
   float y, p, m;
   vec2 coords;
   const float range = 0.3;

   colour_out.a *= beamfade( pos_px.x, pos_tex.x );

   coords = pos_px / 500.0 + vec2( -3.0*ANIM_SPEED*dt, 0.0 );
   m = 1.5 + 0.5*snoise( coords );
   p = 2.0*M_PI * (pos_px.x/20.0 - 8.0*ANIM_SPEED*dt + r);
   y = pos_tex.y + 0.2 * sin( p );
   colour_out.a *= smoothbeam( y, m );
   colour_out.rgb = mix( colour.rgb, vec3(1.0), 4.0*smoothbeam( y, 0.1 ) );

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   float v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   colour_out.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );
}
