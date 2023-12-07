#include "lib/simplex.glsl"
#include "lib/blend.glsl"
#include "lib/blur.glsl"

uniform vec3 dimensions;
uniform sampler2D u_tex;

uniform float u_r;
uniform float u_timer;
uniform float u_elapsed;

const float FADE  = 0.3;

in vec2 tex_coord;
out vec4 colour_out;

void main(void) {
   colour_out = texture( u_tex, tex_coord );
   if (colour_out.a <= 0.0)
      discard;

   float alpha = 1.0 - blur5( u_tex, tex_coord, dimensions.xy, 5.0 ).a;
   alpha *= smoothstep( 0.0, FADE, u_timer );
   alpha *= smoothstep( 0.0, FADE, u_elapsed );

   vec3 coord = vec3( 0.5 * tex_coord * dimensions.xy / dimensions.z, u_elapsed*10.0 + u_r );
   alpha *= 0.5 + 0.5*snoise( coord );

   /* Do the effect. */
   colour_out.rgb = blendGlow( colour_out.rgb, vec3(1.0,0.0,1.0), alpha );
}
