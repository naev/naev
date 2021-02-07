#include "math.h"

// For ideas: https://thebookofshaders.com/05/

uniform vec4 c1;
uniform vec4 c2;
uniform float t1;
uniform float t2;
uniform int type;
uniform float dt;
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

float trail_default( float t, float y )
{
   float a, m;

   // Modulate alpha base on length
   a = fastdropoff( t, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-t, 30. );

   // Modulate width
   a *= smoothbeam( y, 3.*m );

   return a;
}

float trail_pulse( float t, float y )
{
   float a, m, v;

   // Modulate alpha base on length
   a = fastdropoff( t, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-t, 30. );

   // Modulate width
   a *= smoothbeam( y, 3.*m );

   v = smoothstep( 0., 0.5, 1-t );
   a *=  0.8 + 0.2 * mix( 1, sin( 2*M_PI * (t * 25 + dt * 3) ), v );

   return a;
}

float trail_wave( float t, float y )
{
   float a, m, p;

   // Modulate alpha base on length
   a = fastdropoff( t, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-t, 30. );

   // Modulate width
   p = 2*M_PI * (t*5 + dt * 0.5);
   y += 0.2 * smoothstep(0, 0.8, 1-t) * sin( p );
   a *= smoothbeam( y, 2.*m );

   return a;
}

float trail_flame( float t, float y )
{
   float a, m, p;

   // Modulate alpha base on length
   a = fastdropoff( t, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-t, 30. );

   // Modulate width
   // By multiplying two sine waves with different period it looks more like
   // a natural flame.
   p = 2*M_PI * (t*5 + dt * 5);
   y += 0.2 * smoothstep(0, 0.8, 1-t) * sin( p ) * sin( 2.7*p );
   a *= smoothbeam( y, 2.*m );

   return a;
}

float trail_nebula( float t, float y )
{
   float a, m;

   // Modulate alpha base on length
   a = fastdropoff( t, 1.5 );

   // Modulate alpha based on dispersion
   m = impulse( t, 0.3);

   // Modulate width
   m *= 2-smoothstep( 0., 0.2, 1.-t );
   a *= sharpbeam( y, 3*m );
   a *= 0.2 + 0.8*smoothstep( 0., 0.05, 1.-t );

   return a;
}

void main(void) {
   float a, t;

   // Interpolate
   color_out = pos.x*(c2-c1) + c1;
   t = pos.x*(t2-t1) + t1;

   // TODO optimize this with subroutines
   // https://www.khronos.org/opengl/wiki/Shader_Subroutine
   // Main issue is that this would require OpenGL 4.0+ unless
   // worked around
   if (type==1)
      a = trail_pulse( t, pos.y );
   else if (type==2)
      a = trail_wave( t, pos.y );
   else if (type==3)
      a = trail_flame( t, pos.y );
   else if (type==4)
      a = trail_nebula( t, pos.y );
   else
      a = trail_default( t, pos.y );

   color_out.a *= a;

#include "colorblind.glsl"
}
