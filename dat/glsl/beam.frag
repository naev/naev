#ifdef HAS_GL_ARB_shader_subroutine
#extension GL_ARB_shader_subroutine : require

subroutine vec4 beam_func_prototype( vec4 color, vec2 pos_tex, vec2 dim );
subroutine uniform beam_func_prototype beam_func;

#define BEAM_FUNC_PROTOTYPE  subroutine( beam_func_prototype )

#else /* HAS_GL_ARB_shader_subroutine */

#define BEAM_FUNC_PROTOTYPE

#endif /* HAS_GL_ARB_shader_subroutine */

#include "lib/math.glsl"

uniform vec4 color;
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

BEAM_FUNC_PROTOTYPE
vec4 beam_default( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   if (pos_tex.x < .1) 
      color.a *= pos_tex.x / 0.1;
   else if (pos_tex.x > .8)
      color.a *= 1. - ((pos_tex.x - .8) / .2);

   color.a *= smoothbeam( pos_tex.y, 3. );

   return color;
}

void main(void) {
   vec2 pos_tex, pos_px;

   pos_tex.x = pos.x;
   pos_tex.y = 2. * pos.y - 1.;
   pos_px = pos_tex;

#ifdef HAS_GL_ARB_shader_subroutine
   // Use subroutines
   color_out = beam_func( color, pos_tex, pos_px );
#else /* HAS_GL_ARB_shader_subroutine */
   //* Just use default
   color_out = beam_default( color, pos_tex, pos_px );
#endif /* HAS_GL_ARB_shader_subroutine */

#include "colorblind.glsl"
}
