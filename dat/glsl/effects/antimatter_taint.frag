#include "lib/simplex.glsl"
#include "lib/blend.glsl"
#include "lib/blur.glsl"

uniform vec3 dimensions;
uniform sampler2D u_tex;

uniform float u_r = 0.0;
uniform float u_duration = 0.0;
uniform float u_timer = 0.0;

const float FADE  = 0.3;

in vec2 tex_coord;
out vec4 colour_out;

void main(void) {
   colour_out = texture( u_tex, tex_coord );
   if (colour_out.a <= 0.0)
      return;

   float alpha = 1.0 - blur5( u_tex, tex_coord, dimensions.xy, 5.0 ).a;
   alpha *= smoothstep( 0.0, FADE, u_timer );
   alpha *= 1.0 - smoothstep( u_duration-FADE, u_duration, u_timer );

   /* Do the effect. */
   colour_out.rgb = blendGlow( colour_out.rgb, vec3(1.0,0.0,1.0), alpha );
}
