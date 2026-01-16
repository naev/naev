#include "beam/common.glsl"
#include "lib/simplex.glsl"

void main (void) {
   float y, p, a, c, m, v;
   vec2 coords;
   const float range = 0.3;

   colour_out.a *= beamfade( pos_px.x, pos_tex.x );

   m = 0.6 * (0.8 + 0.2*sin( 2.0*M_PI * 0.5*ANIM_SPEED*dt ) );
   p = 2.0*M_PI * (pos_px.x/40.0 - dt * 6.0 + r);
   // First sine
   y = pos_tex.y + m * sin( p );
   a = smoothbeam( y, 0.8 );
   v = 3.0*smoothbeam( y, 0.1 );
   // Second sine
   y  = pos_tex.y + m * sin( p + M_PI );
   a += smoothbeam( y, 0.8 );
   v += 3.0*smoothbeam( y, 0.1 );
   colour_out.rgb = mix( colour.rgb, vec3(1.0), v );
   colour_out.a *= a;

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   colour_out.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );
}
