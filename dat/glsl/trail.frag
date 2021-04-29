#ifdef HAS_GL_ARB_shader_subroutine
#extension GL_ARB_shader_subroutine : require

subroutine vec4 trail_func_prototype( vec4 color, vec2 pos_tex, vec2 dim );
subroutine uniform trail_func_prototype trail_func;

#define TRAIL_FUNC_PROTOTYPE  subroutine( trail_func_prototype )

#else /* HAS_GL_ARB_shader_subroutine */

#define TRAIL_FUNC_PROTOTYPE

#endif /* HAS_GL_ARB_shader_subroutine */

// Libraries
#include "lib/math.glsl"
#include "lib/simplex.glsl"
#include "lib/cellular.glsl"
#include "lib/perlin.glsl"

// For ideas: https://thebookofshaders.com/05/

uniform vec4 c1;  // Start colour
uniform vec4 c2;  // End colour
uniform float t1; // Start time [0,1]
uniform float t2; // End time [0,1]
uniform float dt; // Current time (in seconds)
uniform vec2 pos1;// Start position
uniform vec2 pos2;// End position
uniform float r;  // Unique value per trail [0,1]
uniform vec3 nebu_col; // Base colour of the nebula, only changes when entering new system

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

/* Similar to smoothbeam, but more k == sharper. */
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
   color.a *=  0.8 + 0.2 * mix( 1, sin( 2*M_PI * (0.06 * pos_px.x + dt * 3) ), v );

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
   p = 2*M_PI * (pos_tex.x*5 + dt * 0.5 + r);
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
   p = 2*M_PI * (pos_tex.x*5 + dt * 5 + r);
   y = pos_tex.y + 0.2 * smoothstep(0, 0.8, 1-pos_tex.x) * sin( p ) * sin( 2.7*p );
   color.a *= smoothbeam( y, 2.*m );

   return color;
}

/* Starts thin and gets wide. */
TRAIL_FUNC_PROTOTYPE
vec4 trail_nebula( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   const float SCALAR = pow(2., 4./3. );
   float m, f;
   vec2 coords;

   color.rgb = nebu_col;

   // Modulate alpha base on length
   color.a *= fastdropoff( pos_tex.x, 1 );

   // Modulate alpha based on dispersion
   m = impulse( pos_tex.x, 0.3);

   // Modulate width
   m *= 2-smoothstep( 0., 0.2, 1.-pos_tex.x );
   color.a *= sharpbeam( pos_tex.y, 3*m );
   color.a *= 0.2 + 0.8*smoothstep( 0., 0.05, 1.-pos_tex.x );

   // We only do two iterations here (turbulence noise)
   coords = 0.02 * pos_px + vec2( dt, 0 ) + 1000.*r;
   f  = abs( cnoise( coords * SCALAR ) );
   f += abs( cnoise( coords * pow(SCALAR,2.) ) );
   color.a *= 0.5 + 0.7*f;

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

   // Create three beams with varying parameters
   ncoord = vec2( 0.03 * pos_px.x, 7*dt ) + 1000 * r;
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
   const float speed = 16.;   // How fast the trail moves
   const float scale = 0.13;  // Noise scaling (sharpness)
   float m, p;

   // Modulate alpha base on length
   color.a *= fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 3. );

   // Compute the noise
   p = 1 - 0.7*cellular2x2( scale * pos_px + vec2( speed*dt, 0 ) + 1000. * r ).x;

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
}
