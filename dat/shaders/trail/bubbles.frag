#include "trail/common.glsl"

#include "lib/cellular.glsl"

vec4 trail_func( vec4 colour, vec2 pos_tex, vec2 pos_px )
{
   const float speed = 16.0;   // How fast the trail moves
   const float scale = 0.13;  // Noise scaling (sharpness)
   float m, p;

   // Modulate alpha base on length
   colour.a *= fastdropoff( pos_tex.x, 1.0 );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.0-pos_tex.x, 3.0 );

   // Compute the noise
   p = 1.0 - 0.7*cellular2x2( scale * pos_px + vec2( speed*dt, 0.0 ) + 1000.0 * r ).x;

   // Modulate width
   colour.a   *= p * smoothbeam( pos_tex.y, 2.0*m );
   colour.rgb *= 1.0 + max(0.0, 10.0*(p-0.8));

   return colour;
}
