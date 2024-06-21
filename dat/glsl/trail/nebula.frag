#include "trail/common.glsl"

#include "lib/perlin.glsl"

vec4 trail_func( vec4 colour, vec2 pos_tex, vec2 pos_px )
{
   const float SCALAR = pow(2., 4.0/3.0 );
   float m, f;
   vec2 coords;

   colour.rgb = nebu_col;

   // Modulate alpha base on length
   colour.a *= fastdropoff( pos_tex.x, 1.0 );

   // Modulate alpha based on dispersion
   m = impulse( pos_tex.x, 0.3);

   // Modulate width
   m *= 2.0-smoothstep( 0.0, 0.2, 1.0-pos_tex.x );
   colour.a *= sharpbeam( pos_tex.y, 5.0*m );
   colour.a *= 0.2 + 0.8*smoothstep( 0.0, 0.05, 1.0-pos_tex.x );

   // We only do two iterations here (turbulence noise)
   coords = 0.02 * pos_px + vec2( dt, 0.0 ) + 1000.0*r;
   f  = abs( cnoise( coords * SCALAR ) );
   f += abs( cnoise( coords * pow(SCALAR,2.0) ) );
   colour.a *= 0.5 + 0.7*f;

   // Smoother end trails
   if (pos_tex.x < 0.1) {
      float offy = pow(abs(pos_tex.y),2.0);
      colour.a *= smoothstep( offy, 1.0, pow(pos_tex.x / 0.1, 0.5) );
   }

   return colour;
}
