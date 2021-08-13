#include "lib/nebula.glsl"
#include "lib/sdf.glsl"

uniform vec2 dimensions;
uniform float progress;
out vec4 color_out;
in vec2 pos;

void main(void) {
   vec2 rel_pos = gl_FragCoord.xy * 0.05;

   const float time  = 0.0;
   const float hue   = 0.8;
   float value       = 0.6*step( pos.x, progress );
   float brightness  = 0.1*step( progress, pos.x );

   color_out = nebula( vec4(0.0, 0.0, 0.0, 1.0), rel_pos, time, hue, value, brightness );

   if (pos.x > progress)
      color_out.a *= 0.2;

   const float b = 5.0;
   float dist = sdBox( (pos.xy*2.0-1.0)*dimensions, vec2(dimensions.x-2.0*b,dimensions.y-2.0*b) );
   dist =1.0 - dist / b * 0.5;
   color_out.a *= dist;
}
