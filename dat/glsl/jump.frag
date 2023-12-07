#ifdef HAS_GL_ARB_shader_subroutine
#extension GL_ARB_shader_subroutine : require

subroutine vec4 jump_func_prototype (void);
subroutine uniform jump_func_prototype jump_func;

#define JUMP_FUNC_PROTOTYPE  subroutine( jump_func_prototype )

#else /* HAS_GL_ARB_shader_subroutine */

#define JUMP_FUNC_PROTOTYPE

#endif /* HAS_GL_ARB_shader_subroutine */

// Libraries
#include "lib/math.glsl"
#include "lib/perlin.glsl"
#include "lib/cellular.glsl"

uniform float progress;
uniform float direction;
uniform vec2 dimensions;
uniform float brightness;
in vec2 pos;
out vec4 color_out;

/**
 * Calculates the transform that the player is going through.
 */
vec2 calculate_transform( float size, float smoothness )
{
   const vec2 center = vec2( 0.5, 0.5 );

   // Create rotation matrix
   float s = sin(direction);
   float c = cos(direction);
   float l = sqrt( s*s + c*c );
   mat2 R = mat2( c, -s, s, c );

   // Compute new scaled coordinates
   vec2 lpos = pos * dimensions / dimensions.x;
   lpos = l * (1.0-2.0*(smoothness+size)) * lpos + smoothness + size;
   vec2 uv = R * (lpos - center) + center;

   return uv;
}

/**
 * Does smooth interpolation based on the noise provided and smoothness factor.
 */
vec4 smooth_interpolation( float smoothness, float noise )
{
   vec4 colour_from  = vec4( brightness, brightness, brightness, 0.0 );
   vec4 colour_to    = vec4( brightness, brightness, brightness, 1.0 );

   // Magic!
   float p = mix( -smoothness, 1.0 + smoothness, 1.0-progress );
   float lower = p - smoothness;
   float higher = p + smoothness;
   float q = smoothstep( lower, higher, noise );

   return mix( colour_from, colour_to, q );
}

/* Original Naev jump function in a shader. */
JUMP_FUNC_PROTOTYPE
vec4 jump_default (void)
{
   vec4 colour_from  = vec4( brightness, brightness, brightness, 0.0 );
   vec4 colour_to    = vec4( brightness, brightness, brightness, 1.0 );
   return mix( colour_from, colour_to, progress );
}

JUMP_FUNC_PROTOTYPE
vec4 jump_organic (void)
{
   const float scale       = 20.0;
   const float smoothness  = 0.1;
   float n = cellular2x2( pos * scale ).x;
   return smooth_interpolation( smoothness, n );
}

JUMP_FUNC_PROTOTYPE
vec4 jump_circular (void)
{
   vec4 colour_from        = vec4( brightness, brightness, brightness, 0.0 );
   vec4 colour_to          = vec4( brightness, brightness, brightness, 1.0 );
   const float smoothness  = 0.3;
   const vec2 center       = vec2( 0.5, 0.5 );

   float m = smoothstep(-smoothness, 0.0, M_SQRT2 * distance(center, pos) - (1.0-progress)*(1.0+smoothness));
   return mix( colour_from, colour_to, m );
}

JUMP_FUNC_PROTOTYPE
vec4 jump_nebula (void)
{
   const float size        = 0.2;
   const float scale       = 4.0;
   const float smoothness  = 0.1;

   // Get the transformed smoothed coordinates.
   vec2 uv = calculate_transform( size, smoothness );

   // Compute noise
   float n = uv.x + 2.0*size*abs( cnoise( pos * scale ) );

   // Interpolate and return
   return smooth_interpolation( smoothness, n );
}

JUMP_FUNC_PROTOTYPE
vec4 jump_wind (void)
{
   const float size        = 0.1;
   const float smoothness  = 0.1;
   const float jaggedness  = 200.0;
   const vec2 center       = vec2( 0.5, 0.5 );

   // Get the transformed smoothed coordinates.
   vec2 uv = calculate_transform( size, smoothness );

   // Compute noise
   float n = uv.x + 2.0*size*cnoise( jaggedness * vec2( 0.0, uv.y ) );

   // Interpolate and return
   return smooth_interpolation( smoothness, n );
}

void main(void)
{
#ifdef HAS_GL_ARB_shader_subroutine
   // Use subroutines
   color_out = jump_func();
#else /* HAS_GL_ARB_shader_subroutine */
   //* Just use default
   color_out = jump_wind();
#endif /* HAS_GL_ARB_shader_subroutine */
}
