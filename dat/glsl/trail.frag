#include "math.h"
#include "noise2D.glsl"

// For ideas: https://thebookofshaders.com/05/

uniform vec4 c1;
uniform vec4 c2;
uniform float t1;
uniform float t2;
uniform int type;
uniform float dt;
uniform vec2 dimensions;
in vec2 pos;
out vec4 color_out;

/* Has a peak at 1/k */
float impulse( float x, float k )
{
   float h = x*k;
   return h * exp( 1.0 - h );
}

float fastdropoff( float x, float k )
{
   return 1. - pow( max(0.0, abs(1.-x) * 2.0 - 1.0 ), k );
}

/* k is the sharpness, more k == smoother
 * Good default is 3.0 */
float smoothbeam( float x, float k )
{
   return 1. - pow( abs( sin( M_PI * x / 2. ) ), k );
}

/* Similar to smoothbeam, but morke k == sharper. */
float sharpbeam( float x, float k )
{
   return pow( min( cos( M_PI * x / 2. ), 1.0 - abs(x) ), k );
}

float random (vec2 st) {
   return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

/* No animation. */
float trail_default( vec2 pos_tex, vec2 dim )
{
   float a, m;

   // Modulate alpha base on length
   a = fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 30. );

   // Modulate width
   a *= smoothbeam( pos_tex.y, 3.*m );

   return a;
}

/* Pulsating motion. */
float trail_pulse( vec2 pos_tex, vec2 dim )
{
   float a, m, v;

   // Modulate alpha base on length
   a = fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 30. );

   // Modulate width
   a *= smoothbeam( pos_tex.y, 3.*m );

   v = smoothstep( 0., 0.5, 1-pos_tex.x );
   a *=  0.8 + 0.2 * mix( 1, sin( 2*M_PI * (pos_tex.x * 0.03 * dim.x + dt * 3) ), v );

   return a;
}

/* Slow ondulating wave-like movement. */
float trail_wave( vec2 pos_tex, vec2 dim )
{
   float a, m, p, y;

   // Modulate alpha base on length
   a = fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 30. );

   // Modulate width
   p = 2*M_PI * (pos_tex.x*5 + dt * 0.5);
   y = pos_tex.y + 0.2 * smoothstep(0, 0.8, 1-pos_tex.x) * sin( p );
   a *= smoothbeam( y, 2.*m );

   return a;
}

/* Flame-like periodic movement. */
float trail_flame( vec2 pos_tex, vec2 dim )
{
   float a, m, p, y;

   // Modulate alpha base on length
   a = fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 30. );

   // Modulate width
   // By multiplying two sine waves with different period it looks more like
   // a natural flame.
   p = 2*M_PI * (pos_tex.x*5 + dt * 5);
   y = pos_tex.y + 0.2 * smoothstep(0, 0.8, 1-pos_tex.x) * sin( p ) * sin( 2.7*p );
   a *= smoothbeam( y, 2.*m );

   return a;
}

/* Starts thin and gets wide. */
float trail_nebula( vec2 pos_tex, vec2 dim )
{
   float a, m;

   // Modulate alpha base on length
   a = fastdropoff( pos_tex.x, 1 );

   // Modulate alpha based on dispersion
   m = impulse( pos_tex.x, 0.3);

   // Modulate width
   m *= 2-smoothstep( 0., 0.2, 1.-pos_tex.x );
   a *= sharpbeam( pos_tex.y, 3*m );
   a *= 0.2 + 0.8*smoothstep( 0., 0.05, 1.-pos_tex.x );

   return a;
}

/* Somewhat like a lightning arc. */
float trail_arc( vec2 pos_tex, vec2 dim )
{
   float a, m, p, v, s;
   vec2 ncoord;

   // Modulate alpha base on length
   a = fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   //m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 1. );
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 1. );
   m *= 3;

   // Modulate width
   ncoord = vec2( pos_tex.x * 0.03*dim.x, 7*dt );
   s =  0.6 * smoothstep(0, 0.2, 1.-pos_tex.x);
   p = pos_tex.y + s * snoise( ncoord );
   v = sharpbeam( p, m );
   p = pos_tex.y + s * snoise( 1.5*ncoord );
   v += sharpbeam( p, 2*m );
   p = pos_tex.y + s * snoise( 2*ncoord );
   v += sharpbeam( p, 4*m );

   a *= v * 0.6;

   return min(1, a);
}

/* Bubbly effect. */
float trail_bubbles( vec2 pos_tex, vec2 dim )
{
   float a, m, p;
   vec2 coords;

   // Modulate alpha base on length
   a = fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 3. );

   coords = dim * pos_tex + vec2( 220*dt, 0 );;
   p = 0.5 + min( 0.5, snoise( 0.08 * coords ));

   // Modulate width
   a *= p * smoothbeam( pos_tex.y, 3.*m );

   return a;
}

void main(void) {
   float a;
   vec2 pos_tex;

   // Interpolate
   color_out = mix( c1, c2, pos.x );
   pos_tex   = pos;
   pos_tex.x = mix( t1, t2, pos.x );

   // TODO optimize this with subroutines
   // https://www.khronos.org/opengl/wiki/Shader_Subroutine
   // Main issue is that this would require OpenGL 4.0+ unless
   // worked around
   if (type==1)
      a = trail_pulse( pos_tex, dimensions );
   else if (type==2)
      a = trail_wave( pos_tex, dimensions );
   else if (type==3)
      a = trail_flame( pos_tex, dimensions );
   else if (type==4)
      a = trail_nebula( pos_tex, dimensions );
   else if (type==5)
      a = trail_arc( pos_tex, dimensions );
   else if (type==6)
      a = trail_bubbles( pos_tex, dimensions );
   else
      a = trail_default( pos_tex, dimensions );

   color_out.a *= a;

#include "colorblind.glsl"
}
