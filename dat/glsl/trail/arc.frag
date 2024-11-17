#include "trail/common.glsl"

#include "lib/simplex.glsl"

vec4 trail_func( vec4 colour, vec2 pos_tex, vec2 pos_px )
{
   float m, p, v, s;
   vec2 ncoord;

   // Modulate alpha base on length
   colour.a *= fastdropoff( pos_tex.x, 1.0 );

   // Modulate alpha based on dispersion
   m = 1.5 + 5.0*impulse( 1.0-pos_tex.x, 1.0 );

   // Create three beams with varying parameters
   ncoord = vec2( 0.03 * pos_px.x, 7.0*dt ) + 1000.0 * r;
   s =  0.6 * smoothstep(0.0, 0.2, 1.0-pos_tex.x);
   p = clamp( pos_tex.y + s * snoise( ncoord ), -1.0, 1.0 );
   v = sharpbeam( p, m );
   p = clamp( pos_tex.y + s * snoise( 1.5*ncoord ), -1.0, 1.0 );
   v += sharpbeam( p, 2.0*m );
   p = clamp( pos_tex.y + s * snoise( 2.0*ncoord ), -1.0, 1.0 );
   v += sharpbeam( p, 4.0*m );

   v = abs(v);
   s = s + 0.1;
   colour.rgb  = mix( colour.rgb, vec3(1.0), pow(s*v*0.8, 3.0) );
   colour.a   *= min(1.0, v);

   return colour;
}
