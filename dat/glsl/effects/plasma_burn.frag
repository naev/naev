#include "lib/simplex.glsl"
#include "lib/blend.glsl"

uniform vec3 dimensions;
uniform float dt;
uniform sampler2D u_tex;

uniform float u_duration;
uniform float u_timer;

const float FADE  = 0.3;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   colour_out = texture( u_tex, pos );
   if (colour_out.a <= 0.0)
      return;

   /* Smooth transitions from on/off. */
   float alpha = 0.5;
   alpha *= smoothstep( 0.0, FADE, u_timer );
   alpha *= 1.0 - smoothstep( u_duration-FADE, u_duration, u_timer );

   vec3 overlay = vec3( 0.5, 0.0, 0.0 );

   /* Do the effect. */
   colour_out.rgb = blendGlow( colour_out.rgb, overlay, alpha );
}
