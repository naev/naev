#include "trail/common.glsl"

#include "lib/simplex.glsl"

vec4 trail_func( vec4 colour, vec2 pos_tex, vec2 pos_px )
{
   float m, v, y, p, s;
   vec2 ncoord;

   // Modulate alpha base on length
   colour.a *= fastdropoff( pos_tex.x, 1.0 );

   // Modulate alpha based on dispersion
   m = 1.5 + 5.0*impulse( 1.0-pos_tex.x, 1.0 );

   // Create three beams with varying parameters
   ncoord = vec2( 0.03 * pos_px.x, 5.0*dt ) + 1000.0 * r;
   s =  0.6 * smoothstep(0.0, 0.175, 1.0-pos_tex.x);
   p = 2.0*M_PI * (pos_tex.x*5.0 + dt * 0.5 + r + (snoise( ncoord ) - 0.5)/8.0);
   y = clamp( pos_tex.y + s * sin( p ), -1.0, 1.0 );
   v += sharpbeam( y, m );
   p = 2.0*M_PI * (pos_tex.x*5.0 + dt * 0.5 + r + 1.0/3.0 + (snoise( 1.5*ncoord ) - 0.5)/8.0);
   y = clamp( pos_tex.y + s * sin( p ), -1.0, 1.0 );
   v += sharpbeam( y, m );
   p = 2.0*M_PI * (pos_tex.x*5.0 + dt * 0.5 + r + 2.0/3.0 + (snoise( 2.0*ncoord ) - 0.5)/8.0);
   y = clamp( pos_tex.y + s * sin( p ), -1.0, 1.0 );
   v += sharpbeam( y, m );

   // Modulate width
   v = abs(v);
   colour.a *= min(1.0, smoothbeam( pos_tex.y, m ) * (0.5 + 0.5*v));
   colour.rgb  = mix( colour.rgb, vec3(1.0), min(pow((1.0-s)*v*0.4, 2.0), 0.25) );

   return colour;
}
