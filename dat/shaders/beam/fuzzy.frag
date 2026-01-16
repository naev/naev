
#include "beam/common.glsl"
#include "lib/simplex.glsl"
#include "lib/perlin.glsl"

void main (void) {
   vec2 coords;
   float m, f;
   const float range = 0.3;

   // Normal beam
   coords = vec2( 3.0*ANIM_SPEED*dt, 0.0 );
   m = 4.0 + snoise( coords );
   float a = smoothbeam( pos_tex.y, m );
   colour_out.rgb = colour.rgb + 3.0 * a * smoothbeam( pos_tex.y, 0.1 );
   colour_out.a *= a;
   colour_out.a *= beamfade( pos_px.x, pos_tex.x );

   // Perlin noise
   coords = 0.2 * pos_px + vec2( -45.0*ANIM_SPEED*dt, 0.0 ) + 1000.0*r;
   f = cnoise( coords );
   colour_out.a *= 1.0 + 1.0 * f * smoothstep( 0.0, 1.0, abs(pos_tex.y) );

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   float v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   colour_out.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );
}
