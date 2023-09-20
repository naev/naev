#include "lib/simplex.glsl"
#include "lib/blend.glsl"
#include "lib/blur.glsl"

uniform vec3 dimensions;
uniform sampler2D u_tex;

uniform float u_r;
uniform float u_timer;
uniform float u_elapsed;

in vec2 tex_coord;
out vec4 colour_out;

const vec3 COLOUR    = vec3( 1.0, 0.8, 0.0 );

void main (void)
{
   vec4 blur = blur13( u_tex, tex_coord, dimensions.xy, 3.0 );
   if (blur.a <= 0.0) /* assume u_tex will also have .a <= 0.0 */
      discard;

   vec4 tex = texture( u_tex, tex_coord );
   colour_out = tex;

   float f = clamp( min( u_timer, u_elapsed ), 0.0, 1.0 );
   vec2 nuv = vec2( 0.05 * tex_coord + 0.1 * vec2(u_elapsed,u_r) );
   blur.rgb = blendGlow( blur.rgb, COLOUR, 0.3+0.3*snoise(nuv) );
   colour_out.rgb += f * blur.rgb;
   colour_out.a = max( colour_out.a, blur.a );

   /* 0.2 s fade in/out */
   float m = clamp( 5.0*min( u_timer, u_elapsed ), 0.0, 1.0 );
   colour_out = mix( tex, colour_out, m );
}
