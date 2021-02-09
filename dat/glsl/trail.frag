#ifdef HAS_GL_ARB_shader_subroutine
#extension GL_ARB_shader_subroutine : require

subroutine vec4 trail_func_prototype( vec4 color, vec2 pos_tex, vec2 dim );
subroutine uniform trail_func_prototype trail_func;

#define TRAIL_FUNC_PROTOTYPE  subroutine( trail_func_prototype )
#else /* HAS_GL_ARB_shader_subroutine */
#define TRAIL_FUNC_PROTOTYPE
#endif /* HAS_GL_ARB_shader_subroutine */


#include "lib/math.glsl"
#include "lib/simplex2D.glsl"
#include "lib/cellular2x2.glsl"

// For ideas: https://thebookofshaders.com/05/

uniform vec4 c1;
uniform vec4 c2;
uniform float t1;
uniform float t2;
uniform float dt;
uniform vec2 pos1;
uniform vec2 pos2;
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

/* No animation. */
TRAIL_FUNC_PROTOTYPE
vec4 trail_default( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m;

   // Modulate alpha base on length
   color.a *= fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 30. );

   // Modulate width
   color.a *= smoothbeam( pos_tex.y, 3.*m );

   return color;
}

/* Pulsating motion. */
TRAIL_FUNC_PROTOTYPE
vec4 trail_pulse( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m, v;

   // Modulate alpha base on length
   color.a *= fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 30. );

   // Modulate width
   color.a *= smoothbeam( pos_tex.y, 3.*m );

   v = smoothstep( 0., 0.5, 1-pos_tex.x );
   color.a *=  0.8 + 0.2 * mix( 1, sin( 2*M_PI * (0.03 * pos_px.x + dt * 3) ), v );

   return color;
}

/* Slow ondulating wave-like movement. */
TRAIL_FUNC_PROTOTYPE
vec4 trail_wave( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m, p, y;

   // Modulate alpha base on length
   color.a *= fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 30. );

   // Modulate width
   p = 2*M_PI * (pos_tex.x*5 + dt * 0.5);
   y = pos_tex.y + 0.2 * smoothstep(0, 0.8, 1-pos_tex.x) * sin( p );
   color.a *= smoothbeam( y, 2.*m );

   return color;
}

/* Flame-like periodic movement. */
TRAIL_FUNC_PROTOTYPE
vec4 trail_flame( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m, p, y;

   // Modulate alpha base on length
   color.a *= fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 30. );

   // Modulate width
   // By multiplying two sine waves with different period it looks more like
   // a natural flame.
   p = 2*M_PI * (pos_tex.x*5 + dt * 5);
   y = pos_tex.y + 0.2 * smoothstep(0, 0.8, 1-pos_tex.x) * sin( p ) * sin( 2.7*p );
   color.a *= smoothbeam( y, 2.*m );

   return color;
}

/* Starts thin and gets wide. */
TRAIL_FUNC_PROTOTYPE
vec4 trail_nebula( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m;

   // Modulate alpha base on length
   color.a *= fastdropoff( pos_tex.x, 1 );

   // Modulate alpha based on dispersion
   m = impulse( pos_tex.x, 0.3);

   // Modulate width
   m *= 2-smoothstep( 0., 0.2, 1.-pos_tex.x );
   color.a *= sharpbeam( pos_tex.y, 3*m );
   color.a *= 0.2 + 0.8*smoothstep( 0., 0.05, 1.-pos_tex.x );

   return color;
}

/* Somewhat like a lightning arc. */
TRAIL_FUNC_PROTOTYPE
vec4 trail_arc( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m, p, v, s;
   vec2 ncoord;

   // Modulate alpha base on length
   color.a *= fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   m = 1.5 + 1.5*impulse( 1.-pos_tex.x, 1. );

   // Modulate width
   ncoord = vec2( 0.03 * pos_px.x, 7*dt );
   s =  0.6 * smoothstep(0, 0.2, 1.-pos_tex.x);
   p = pos_tex.y + s * snoise( ncoord );
   v = sharpbeam( p, m );
   p = pos_tex.y + s * snoise( 1.5*ncoord );
   v += sharpbeam( p, 2*m );
   p = pos_tex.y + s * snoise( 2*ncoord );
   v += sharpbeam( p, 4*m );

   color.xyz *= 1 + max(0, 3*(v-0.9));
   color.a   *= min(1, 0.6*v);

   return color;
}

/* Bubbly effect. */
TRAIL_FUNC_PROTOTYPE
vec4 trail_bubbles( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m, p;
   vec2 coords;

   // Modulate alpha base on length
   color.a *= fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 3. );

   //coords = pos_px + vec2( 220*dt, 0 );
   //p = 0.5 + min( 0.5, snoise( 0.08 * coords ));
   coords = pos_px + vec2( 120*dt, 0 );
   p = 1 - 0.7*cellular2x2( 0.13 * coords ).x;

   // Modulate width
   color.a   *= p * smoothbeam( pos_tex.y, 3.*m );
   color.xyz *= 1 + max(0, 10*(p-0.8));

   return color;
}

void main(void) {
   vec2 pos_tex, pos_px;

   // Interpolate
   color_out = mix( c1, c2, pos.x );
   pos_px    = mix( pos1, pos2, pos );
   pos_px.y *= pos.y;
   pos_tex.x = mix( t1, t2, pos.x );
   pos_tex.y = 2. * pos.y - 1.;

#ifdef HAS_GL_ARB_shader_subroutine
   // Use subroutines
   color_out = trail_func( color_out, pos_tex, pos_px );
#else /* HAS_GL_ARB_shader_subroutine */
   //* Just use default
   color_out = trail_default( color_out, pos_tex, pos_px );
#endif /* HAS_GL_ARB_shader_subroutine */

#include "colorblind.glsl"
}
