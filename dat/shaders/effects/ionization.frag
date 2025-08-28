#include "lib/simplex.glsl"
#include "lib/blend.glsl"
#include "lib/blur.glsl"

uniform vec3 dimensions;
uniform sampler2D u_tex;

uniform float u_r       = 0.0;
uniform float u_timer   = 0.0;
uniform float u_elapsed = 0.0;

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
   alpha *= 0.4 + 0.4*snoise( coord );

   /* Do the effect. */
   colour_out.rgb = mix( colour_out.rgb, vec3(0.4,0.6,0.95), alpha );
}
