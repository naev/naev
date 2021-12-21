#include "lib/simplex.glsl"
#include "lib/blend.glsl"

uniform vec3 dimensions;
uniform sampler2D u_tex;

uniform float u_r = 0.0;
uniform float u_duration = 0.0;
uniform float u_timer = 0.0;

const float FADE  = 0.3;
const vec3 COLOUR = vec3( 0.5, 0.0, 0.0 );

in vec2 pos;
out vec4 colour_out;

void main(void) {
   colour_out = texture( u_tex, pos );
   if (colour_out.a <= 0.0)
      return;

   /* Smooth transitions from on/off. */
   float alpha = 0.75;
   alpha *= smoothstep( 0.0, FADE, u_timer );
   alpha *= 1.0 - smoothstep( u_duration-FADE, u_duration, u_timer );

   vec3 coord = vec3( 0.02 * (2.0*pos-1.0) * dimensions.xy / dimensions.z, u_timer + u_r );
   alpha *= 0.75 + 0.5*snoise( coord );

   /* Do the effect. */
   colour_out.rgb = blendGlow( colour_out.rgb, COLOUR, alpha );
}
