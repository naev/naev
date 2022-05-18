#include "lib/simplex.glsl"
#include "lib/blend.glsl"
#include "lib/blur.glsl"

uniform vec3 dimensions;
uniform sampler2D u_tex;

uniform float u_r       = 0.0;
//uniform float u_timer   = 0.0;
uniform float u_elapsed = 0.0;

in vec2 tex_coord;
out vec4 colour_out;

const vec3 COLOUR    = vec3( 1.0, 0.6, 1.0 );

void main(void)
{
   vec4 blur = blur5( u_tex, tex_coord, dimensions.xy, 2.5 );
   if (blur.a <= 0.0) /* assume u_tex will also have .a <= 0.0 */
      discard;

   colour_out = texture( u_tex, tex_coord );
   vec3 coord = vec3( 0.03 * tex_coord * dimensions.xy / dimensions.z, u_elapsed*0.7 + u_r );
   blur.rgb = blendGlow( blur.rgb, COLOUR, 0.3+0.2*snoise(coord) );
   colour_out.rgb = blendGlow( colour_out.rgb, COLOUR, 0.5 );
   colour_out.rgb = mix( blur.rgb, colour_out.rgb, min( 0.3, colour_out.a) ) + 0.1;
   colour_out.a = max( colour_out.a, blur.a );
}
