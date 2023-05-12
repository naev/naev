#include "lib/nebula.glsl"

uniform float hue;
uniform float nonuniformity;
uniform mat4 projection;
uniform float eddy_scale;
uniform float time;
uniform float volatility;
in vec4 base_col;
out vec4 colour_out;

vec4 nebula_default (void)
{
   vec2 rel_pos = gl_FragCoord.xy + projection[3].xy;
   rel_pos /= eddy_scale;
   return nebula( vec4(0.0, 0.0, 0.0, 1.0), rel_pos, time, hue, 1.0, volatility, 0.1 );
}

void main (void)
{
   /* Case disabled. */
   if (nonuniformity <= 0.0) {
      colour_out = base_col;
      return;
   }

   //* Just use default
   colour_out = nebula_default();

   if (nonuniformity < 1.0) {
      colour_out = mix( base_col, colour_out, nonuniformity );
   }
}
