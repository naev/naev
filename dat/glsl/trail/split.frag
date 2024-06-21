#include "trail/common.glsl"

vec4 trail_func( vec4 colour, vec2 pos_tex, vec2 pos_px )
{
   // Modulate alpha base on length
   colour.a *= fastdropoff( pos_tex.x, 1.0 );

   // Modulate alpha based on dispersion
   float m = 1.5 + 5.0*impulse( 1.0-pos_tex.x, 1.0 );

   // Modulate width
   colour.a *= smoothbeam( pos_tex.y, m );

   // Split the tail
   colour.a -= max( 0.0, (0.9 - pos_tex.x) * smoothbeam( pos_tex.y, 0.2 ) );

   return colour;
}
