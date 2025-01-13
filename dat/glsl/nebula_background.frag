#include "lib/nebula.glsl"

layout(std140) uniform NebulaData {
   float hue;
   float horizon;
   float eddy_scale;
   float elapsed;
   float nonuniformity;
   float volatility;
   float saturation;
   vec2 camera;
};

in vec4 base_col;
layout(location = 0) out vec4 colour_out;

vec4 nebula_default (void)
{
   vec2 rel_pos = gl_FragCoord.xy - camera;
   rel_pos /= eddy_scale;
   return nebula( vec4(0.0, 0.0, 0.0, 1.0), rel_pos, elapsed, hue, saturation, volatility, 0.1 );
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
