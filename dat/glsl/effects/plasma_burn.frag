#include "lib/simplex.glsl"
#include "lib/blend.glsl"

uniform vec3 dimensions;
uniform sampler2D sampler;

uniform float u_duration;
uniform float u_timer;
uniform float u_dt;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   colour_out = texture( sampler, pos );
   if (colour_out.a <= 0.0)
      return;

   vec3 overlay = vec3( 0.5, 0.0, 0.0 );

   /* Do the effect. */
   colour_out.rgb = blendGlow( colour_out.rgb, overlay, 0.5 );
}
