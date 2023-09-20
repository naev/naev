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

const vec3 COLOUR    = vec3( 0.26, 0.56, 0.86 );

void main(void)
{
   vec4 blur = blur5( u_tex, tex_coord, dimensions.xy, 1.45 );
   if (blur.a <= 0.0) /* assume u_tex will also have .a <= 0.0 */
      discard;

   float intensity = max(0.02, 1.0 - u_elapsed*0.2);
   vec4 texin = texture( u_tex, tex_coord );
   colour_out = texin;
   vec3 coord = vec3( 0.12 * tex_coord * dimensions.xy / dimensions.z, u_elapsed*0.67 + u_r );
   blur.rgb = blendReflect( blur.rgb, COLOUR, min(intensity, 0.06+0.4*snoise(coord)) );
   colour_out.rgb = blendScreen( blur.rgb, colour_out.rgb, min( 0.3 , colour_out.a) ) + min(intensity, 0.024);
   colour_out.rgb = blendSoftLight( colour_out.rgb, COLOUR, intensity );
   colour_out.a = max( colour_out.a, blur.a );

   /* Smooth fade out. */
   colour_out = mix( texin, colour_out, min( 1.0, min( u_elapsed*2.0, u_timer ) ) );
}
