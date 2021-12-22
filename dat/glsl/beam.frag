#ifdef HAS_GL_ARB_shader_subroutine
#extension GL_ARB_shader_subroutine : require

subroutine vec4 beam_func_prototype( vec4 color, vec2 pos_tex, vec2 dim );
subroutine uniform beam_func_prototype beam_func;

#define BEAM_FUNC_PROTOTYPE  subroutine( beam_func_prototype )

#else /* HAS_GL_ARB_shader_subroutine */

#define BEAM_FUNC_PROTOTYPE

#endif /* HAS_GL_ARB_shader_subroutine */

// Libraries
#include "lib/math.glsl"
#include "lib/simplex.glsl"
#include "lib/cellular.glsl"
#include "lib/perlin.glsl"

// For ideas: https://thebookofshaders.com/05/

uniform vec4 color;
uniform vec2 dimensions;
uniform float dt;
uniform float r;

const float ANIM_SPEED   = 1.0/1.5; /**< Controls the global speed of the animations. */

in vec2 pos;
out vec4 color_out;

/* k is the sharpness, more k == smoother
 * Good default is 3.0 */
float smoothbeam( float x, float k )
{
   return 1.0 - pow( abs( sin( M_PI * x / 2.0 ) ), k );
}

/* Similar to smoothbeam, but more k == sharper. */
float sharpbeam( float x, float k )
{
   return pow( min( cos( M_PI * x / 2.0 ), 1.0 - abs(x) ), k );
}

float beamfade( float p, float x )
{
   if (p < 7.0)
      return p / 7.0;
   return 1.0 - smoothstep( 0.0, 0.2, x-0.8 );
}

BEAM_FUNC_PROTOTYPE
vec4 beam_default( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   vec2 coords;
   float m;
   const float range = 0.3;

   color.a *= beamfade( pos_px.x, pos_tex.x );

   // Normal beam
   coords = pos_px / 500.0 + vec2( -3.0*ANIM_SPEED*dt, 0 );
   m = 1.5 + 0.5*snoise( coords );
   float a = smoothbeam( pos_tex.y, m );
   color.rgb = mix( color.rgb, vec3(1.0), 3.0*smoothbeam( pos_tex.y, 0.1 ) );
   color.a *= a;

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   float v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   color.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_unstable( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   vec2 coords;
   float a, m, y;
   const float range = 0.3;

   // Normal beam
   coords = vec2( 3.0*ANIM_SPEED*(dt+r), 0.0 );
   m = 0.3 + 0.3*snoise( coords );

   coords = pos_px/2000.0 + vec2( -3.0*ANIM_SPEED*dt, 1000.0 );
   y = pos_tex.y + 0.2*snoise( coords );
   a = smoothbeam( y, m );
   coords += vec2( 1000., 0.0 );
   y = pos_tex.y + 0.7*snoise( coords );
   a += 0.5*smoothbeam( y, 0.5 );
   coords += vec2( 1000.0, 0.0 );
   y = pos_tex.y + 0.7*snoise( coords );
   a += 0.5*smoothbeam( y, 0.5 );
   a = min( a, 1.0 );
   color.rgb = mix( color.rgb, vec3(1.0), 3.0*smoothbeam( pos_tex.y, 0.1 ) );
   color.a *= a;
   color.a *= beamfade( pos_px.x, pos_tex.x );

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   float v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   color.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_fuzzy( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   vec2 coords;
   float m, f;
   const float range = 0.3;

   // Normal beam
   coords = vec2( 3.0*ANIM_SPEED*dt, 0.0 );
   m = 4.0 + snoise( coords );
   float a = smoothbeam( pos_tex.y, m );
   color.rgb += 3.0 * a * smoothbeam( pos_tex.y, 0.1 );
   color.a *= a;
   color.a *= beamfade( pos_px.x, pos_tex.x );

   // Perlin noise
   coords = 0.2 * pos_px + vec2( -45.0*ANIM_SPEED*dt, 0.0 ) + 1000.0*r;
   f = cnoise( coords );
   color.a *= 1.0 + 1.0 * f * smoothstep( 0.0, 1.0, abs(pos_tex.y) );

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   float v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   color.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_wave( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float y, p, m;
   vec2 coords;
   const float range = 0.3;

   color.a *= beamfade( pos_px.x, pos_tex.x );

   coords = pos_px / 500.0 + vec2( -3.0*ANIM_SPEED*dt, 0.0 );
   m = 1.5 + 0.5*snoise( coords );
   p = 2.0*M_PI * (pos_px.x/20.0 - 8.0*ANIM_SPEED*dt + r);
   y = pos_tex.y + 0.2 * sin( p );
   color.a *= smoothbeam( y, m );
   color.rgb = mix( color.rgb, vec3(1.0), 4.0*smoothbeam( y, 0.1 ) );

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   float v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   color.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_arc( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m, y, v;
   vec2 ncoord;

   // Modulate alpha base on length
   color.a *= beamfade( pos_px.x, pos_tex.x );

   // Modulate alpha based on dispersion
   m = 4.0;

   // Modulate width
   ncoord = vec2( 0.03 * pos_px.x, 7.0*ANIM_SPEED*dt ) + 1000.0 * r;
   y = clamp( pos_tex.y + 0.7 * snoise( ncoord ), -1.0, 1.0 );
   v = sharpbeam( y, m );
   y = clamp( pos_tex.y + 0.7 * snoise( 1.5*ncoord ), -1.0, 1.0 );
   v += sharpbeam( y, 2.0*m );
   y = clamp( pos_tex.y + 0.7 * snoise( 2.0*ncoord ), -1.0, 1.0 );
   v += sharpbeam( y, 4.0*m );

   v = abs(v);
   color.rgb  = mix( color.rgb, vec3(1.0), v*0.5 );
   color.rgb  = pow( color.rgb, vec3(3.0) );
   color.a   *= min(1.0, v);

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_helix( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float y, p, a, c, m, v;
   vec2 coords;
   const float range = 0.3;

   color.a *= beamfade( pos_px.x, pos_tex.x );

   m = 0.6 * (0.8 + 0.2*sin( 2.0*M_PI * 0.5*ANIM_SPEED*dt ) );
   p = 2.0*M_PI * (pos_px.x/40.0 - dt * 6.0 + r);
   // First sine
   y = pos_tex.y + m * sin( p );
   a = smoothbeam( y, 0.8 );
   v = 3.0*smoothbeam( y, 0.1 );
   // Second sine
   y  = pos_tex.y + m * sin( p + M_PI );
   a += smoothbeam( y, 0.8 );
   v += 3.0*smoothbeam( y, 0.1 );
   color.rgb = mix( color.rgb, vec3(1.0), v );
   color.a *= a;

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   color.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_organic( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m, p;
   vec2 coords;
   const float range = 0.3;

   color.a *= beamfade( pos_px.x, pos_tex.x );

   // Modulate alpha based on dispersion
   m = 3.0;

   coords = pos_px + vec2( -320.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   p = 1.0 - 0.7*cellular2x2( 0.13 * coords ).x;

   // Modulate width
   color.a   *= p * smoothbeam( pos_tex.y, m );
   color.rgb = mix( color.rgb, vec3(1.0), max(0.0, 10.0*(p-0.9)) );
   color.a    = pow( color.a, 1.5 );

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   float v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   color.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );

  return color;
}

void main(void) {
   vec2 pos_tex, pos_px;

   pos_tex.x = pos.x;
   pos_tex.y = 2.0 * pos.y - 1.0;
   pos_px = pos * dimensions;

#ifdef HAS_GL_ARB_shader_subroutine
   // Use subroutines
   color_out = beam_func( color, pos_tex, pos_px );
#else /* HAS_GL_ARB_shader_subroutine */
   //* Just use default
   color_out = beam_default( color, pos_tex, pos_px );
#endif /* HAS_GL_ARB_shader_subroutine */
}
