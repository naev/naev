#include "lib/nebula.glsl"

uniform vec2 dimensions;
uniform float progress;
out vec4 color_out;
in vec2 pos;

void main(void) {
   vec2 rel_pos = gl_FragCoord.xy * 0.05;

   const float time  = 0.0;
   const float hue   = 0.8;
   float value = 1.0*step( pos.x, progress );
   float brightness  = 0.1*step( progress, pos.x );

   color_out = nebula( vec4(0.0, 0.0, 0.0, 1.0), rel_pos, time, hue, value, brightness );

   if (pos.x > progress)
      color_out.a *= 0.2;
}
