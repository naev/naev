#include "trail/common.glsl"

vec4 trail_func( vec4 colour, vec2 pos_tex, vec2 pos_px )
{
   float m;

   // Modulate alpha base on length
   colour.a *= fastdropoff( pos_tex.x, 1.0 );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.0-pos_tex.x, 30.0 );

   // Modulate width
   colour.a *= smoothbeam( pos_tex.y, m );

   // Pulse effect
   float v = smoothstep( 0.0, 0.5, 1.0-pos_tex.x );
   colour.rgb += m;
   colour.a *=  0.8 + 0.2 * mix( 1.0, sin( 2.0*M_PI * (0.02 * pos_px.x + dt * 2.0) ), v );

   return colour;
}
