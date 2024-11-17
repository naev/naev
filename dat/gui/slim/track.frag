#include "lib/sdf.glsl"
#include "lib/math.glsl"

uniform float u_track;

const vec2 dimensions = vec2(12.0);

vec4 effect( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec2 uv = texture_coords*2.0-1.0;
   float m = 1.0 / dimensions.x;
   vec2 sc = vec2( sin(u_track*M_PI), cos(u_track*M_PI) );
   float d = sdArc( uv, vec2(1.0,0.0), sc, 0.6, 0.2 );

   if (u_track >= 1.0) {
      d = max( d, -sdBox( uv, vec2( 0.2, 0.9 ) )+m*2.0 );
      d = max( d, -sdBox( uv, vec2( 0.9, 0.2 ) )+m*2.0 );
      colour.a = smoothstep( -m, 0.0, -d );

      d = sdBox( uv, vec2( 0.05, 0.9 ) )-0.1+m;
      //d = min( d, sdBox( uv, vec2( 0.05, 0.9 ) )-0.1+m );
      d = min( d, sdBox( uv, vec2( 0.9, 0.05 ) )-0.1+m );
      colour.a = colour.a * 0.3 + smoothstep( -m, 0.0, -d );
   }
   else
      colour.a = smoothstep( -m, 0.0, -d );

   return colour;
}
