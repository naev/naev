#include "lib/nebula.glsl"

uniform float hue;
uniform float nonuniformity;
uniform mat4 projection;
uniform float eddy_scale;
uniform float time;
uniform float volatility;
out vec4 colour_out;

void main (void)
{
   /* Case disabled. */
   if (nonuniformity <= 0) {
      colour_out = vec4( hsv2rgb( vec3(hue, 1.0, 1.0) ), 1.0 );
      return;
   }

   vec2 rel_pos = gl_FragCoord.xy + projection[3].xy;
   rel_pos /= eddy_scale;
   colour_out = nebula( vec4(0.0, 0.0, 0.0, 1.0), rel_pos, time, hue, 1.0, volatility, 0.1 );
   if (nonuniformity < 1.0) {
      vec4 base = vec4( hsv2rgb( vec3(hue, 1.0, 1.0) ), 1.0 );
      colour_out = mix( base, colour_out, nonuniformity );
   }
}
