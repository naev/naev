#include "trail/common.glsl"

vec4 trail_func( vec4 colour, vec2 pos_tex, vec2 pos_px )
{
   float m, p, y;

   // Modulate alpha base on length
   colour.a *= fastdropoff( pos_tex.x, 1.0 );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.0-pos_tex.x, 30.0 );

   // Modulate width
   // By multiplying two sine waves with different period it looks more like
   // a natural flame.
   p = 2.0*M_PI * (pos_tex.x*5.0 + dt * 3.0 + r);
   y = pos_tex.y + 0.15 * smoothstep(0.0, 0.9, 1.0-2.0*pos_tex.x) * sin( p ) * sin( 2.7*p );
   colour.a *= smoothbeam( y, m );

   float r = smoothstep( 0.2, 0.8, 1.4-pos_tex.x);
   colour.rgb = mix( colour.rgb, vec3( 1.0, 0.84, 0.0 ), r );

   return colour;
}
