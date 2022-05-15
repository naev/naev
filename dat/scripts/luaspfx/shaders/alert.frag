#include "lib/sdf.glsl"

uniform float u_time = 0.0;
uniform float u_size = 100.0;

vec4 effect( vec4 color, Image tex, vec2 uv, vec2 px )
{
   color.a *= sin(u_time*20.0) * 0.1 + 0.9;

   /* Base Alpha */
   float a = step( sin( u_size * (uv.x + uv.y) * 0.3), 0.0);

   /* Signed Distance Function Exclamation Point */
   vec2 p = 2.0*uv-1.0;
   p.y *= -1.0;
   float dc = sdCircle( p, 1.0 );
   p *= 1.2;
   float d = min( sdCircle( p+vec2(0.0,0.65), 0.15), sdUnevenCapsuleY( p+vec2(0,0.15), 0.1, 0.25, 0.7 ));

   /* Add border and make center solid. */
   a *= step( 0.0, d-20.0/u_size );
   a += step( d, 0.0 );

   /* Second border. */
   float off = 15.0 / u_size;
   a *= step( dc+off, 0.0 );
   a += step( -(dc+off), 0.0 );
   a *= step( dc, 0.0 );

   color.a *= a;
   return color;
}
