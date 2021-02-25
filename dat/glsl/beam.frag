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

in vec2 pos;
out vec4 color_out;

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

float beamfade( float x )
{
   /*
   if (x < .1)
      return x / 0.1;
   else if (x > .8)
      return 1. - ((x - .8) / .2);
   return 1.;
   */
   return 1. - smoothstep( 0., 0.2, x-0.8 );
}

BEAM_FUNC_PROTOTYPE
vec4 beam_default( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   vec2 coords;
   float m;
   const float range = 0.3;

   color.a *= beamfade( pos_tex.x );

   // Normal beam
   coords = pos_px / 500. + vec2( -3.*dt, 0 );
   m = 2 + snoise( coords );
   float a = smoothbeam( pos_tex.y, m );
   color.xyz = mix( color.xyz, vec3(1,1,1), 3*smoothbeam( pos_tex.y, 0.1 ) );
   color.a *= a;

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5. ) + vec2( -10.*dt, 0 ) + 1000. * r;
   float v = snoise( coords );
   v = max( 0, v-(1.-range) ) * (2./range) - 0.1;
   color.a += v * (1. - smoothstep( 0., 0.05, pos_tex.x-0.95 ) );

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_unstable( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   vec2 coords;
   float a, m, y;
   const float range = 0.3;

   // Normal beam
   coords = vec2( 3.*(dt+r), 0 );
   m = 0.5 + 0.5*snoise( coords );

   coords = pos_px/2000. + vec2( -3*dt, 1000. );
   y = pos_tex.y + 0.2*snoise( coords );
   a = smoothbeam( y, m );
   coords += vec2( 1000., 0. );
   y = pos_tex.y + 0.7*snoise( coords );
   a += 0.5*smoothbeam( y, 0.5 );
   coords += vec2( 1000., 0. );
   y = pos_tex.y + 0.7*snoise( coords );
   a += 0.5*smoothbeam( y, 0.5 );
   a = min( a, 1 );
   color.xyz = mix( color.xyz, vec3(1,1,1), 3.*smoothbeam( pos_tex.y, 0.1 ) );
   color.a *= a;
   color.a *= beamfade( pos_tex.x );

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5. ) + vec2( -10.*dt, 0 ) + 1000. * r;
   float v = snoise( coords );
   v = max( 0, v-(1.-range) ) * (2./range) - 0.1;
   color.a += v * (1. - smoothstep( 0., 0.05, pos_tex.x-0.95 ) );

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_fuzzy( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   vec2 coords;
   float m, f;
   const float range = 0.3;

   // Normal beam
   coords = vec2( 3.*dt, 0 );
   m = 6 + snoise( coords );
   float a = smoothbeam( pos_tex.y, m );
   color.xyz += 3. * a * smoothbeam( pos_tex.y, 0.1 );
   color.a *= a;
   color.a *= beamfade( pos_tex.x );

   // Perlin noise
   coords = 0.2 * pos_px + vec2( -45.*dt, 0 ) + 1000*r;
   f = abs( cnoise( coords ) );
   color.a *= 1 + f * smoothstep( 0., 1, abs(pos_tex.y) );

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5. ) + vec2( -10.*dt, 0 ) + 1000. * r;
   float v = snoise( coords );
   v = max( 0, v-(1.-range) ) * (2./range) - 0.1;
   color.a += v * (1. - smoothstep( 0., 0.05, pos_tex.x-0.95 ) );

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_wave( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float y, p, m;
   vec2 coords;
   const float range = 0.3;

   color.a *= beamfade( pos_tex.x );

   coords = pos_px / 500. + vec2( -3.*dt, 0 );
   m = 2 + snoise( coords );
   p = 2*M_PI * (pos_px.x/20.- dt * 8. + r);
   y = pos_tex.y + 0.2 * sin( p );
   color.a *= smoothbeam( y, m );
   color.xyz = mix( color.xyz, vec3(1,1,1), 5*smoothbeam( y, 0.1 ) );

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5. ) + vec2( -10*dt, 0 ) + 1000. * r;
   float v = snoise( coords );
   v = max( 0, v-(1.-range) ) * (2./range) - 0.1;
   color.a += v * (1. - smoothstep( 0., 0.05, pos_tex.x-0.95 ) );

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_arc( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m, y, v;
   vec2 ncoord;

   // Modulate alpha base on length
   color.a *= beamfade( pos_tex.x );

   // Modulate alpha based on dispersion
   m = 3.;

   // Modulate width
   ncoord = vec2( 0.03 * pos_px.x, 7*dt ) + 1000 * r;
   y = pos_tex.y + 0.7 * snoise( ncoord );
   v = sharpbeam( y, m );
   y = pos_tex.y + 0.7 * snoise( 1.5*ncoord );
   v += sharpbeam( y, 2*m );
   y = pos_tex.y + 0.7 * snoise( 2*ncoord );
   v += sharpbeam( y, 4*m );

   color.xyz *= 1 + max(0, 2*(v-0.9));
   color.a   *= min(1, 0.6*v);

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_helix( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float y, p, a, c, m, v;
   vec2 coords;
   const float range = 0.3;

   color.a *= beamfade( pos_tex.x );

   m = 0.6 * (0.8 + 0.2*sin( 2*M_PI * (dt*0.5) ) );
   p = 2*M_PI * (pos_px.x/40.- dt * 6. + r);
   y = pos_tex.y + m * sin( p );
   a = smoothbeam( y, 1. );
   v = 3.*smoothbeam( y, 0.1 );
   y = pos_tex.y + m * sin( p + M_PI );
   a += smoothbeam( y, 1. );
   v += 3.*smoothbeam( y, 0.1 );
   color.xyz = mix( color.xyz, vec3(1,1,1), v );
   color.a *= a;

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5. ) + vec2( -10*dt, 0 ) + 1000. * r;
   v = snoise( coords );
   v = max( 0, v-(1.-range) ) * (2./range) - 0.1;
   color.a += v * (1. - smoothstep( 0., 0.05, pos_tex.x-0.95 ) );

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_organic( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m, p;
   vec2 coords;
   const float range = 0.3;

   color.a *= beamfade( pos_tex.x );

   // Modulate alpha based on dispersion
   m = 1;

   coords = pos_px + vec2( -320*dt, 0 ) + 1000 * r;
   p = 1 - 0.7*cellular2x2( 0.13 * coords ).x;

   // Modulate width
   color.a   *= p * smoothbeam( pos_tex.y, 3.*m );
   color.xyz = mix( color.xyz, vec3(1,1,1), max(0, 10*(p-0.9)) );

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5. ) + vec2( -10*dt, 0 ) + 1000. * r;
   float v = snoise( coords );
   v = max( 0, v-(1.-range) ) * (2./range) - 0.1;
   color.a += v * (1. - smoothstep( 0., 0.05, pos_tex.x-0.95 ) );

  return color;
}

void main(void) {
   vec2 pos_tex, pos_px;

   pos_tex.x = pos.x;
   pos_tex.y = 2. * pos.y - 1.;
   pos_px = pos * dimensions;

#ifdef HAS_GL_ARB_shader_subroutine
   // Use subroutines
   color_out = beam_func( color, pos_tex, pos_px );
#else /* HAS_GL_ARB_shader_subroutine */
   //* Just use default
   color_out = beam_default( color, pos_tex, pos_px );
#endif /* HAS_GL_ARB_shader_subroutine */

#include "colorblind.glsl"
}
