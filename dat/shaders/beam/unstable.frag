#include "beam/common.glsl"
#include "lib/simplex.glsl"

void main (void) {
   vec2 coords;
   float a, m, y;
   const float range = 0.3;

   // Normal beam
   coords = vec2( 3.0*ANIM_SPEED*(dt+r), 0.0 );
   m = 0.3 + 0.3*snoise( coords );

   coords = pos_px/2000.0 + vec2( -3.0*ANIM_SPEED*dt, 1000.0 );
   y = pos_tex.y + 0.2*snoise( coords );
   a = smoothbeam( y, m );
   coords += vec2( 1000., 0.0 );
   y = pos_tex.y + 0.7*snoise( coords );
   a += 0.5*smoothbeam( y, 0.5 );
   coords += vec2( 1000.0, 0.0 );
   y = pos_tex.y + 0.7*snoise( coords );
   a += 0.5*smoothbeam( y, 0.5 );
   a = min( a, 1.0 );
   colour_out.rgb = mix( colour.rgb, vec3(1.0), 3.0*smoothbeam( pos_tex.y, 0.1 ) );
   colour_out.a = colour.a * a;
   colour_out.a *= beamfade( pos_px.x, pos_tex.x );

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   float v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   colour_out.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );
}
