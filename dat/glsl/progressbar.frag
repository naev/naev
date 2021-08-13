#include "lib/nebula.glsl"
#include "lib/sdf.glsl"

uniform vec2 dimensions;
uniform float r;
out vec4 color_out;
in vec2 pos;

void main(void) {
   float progress = r;
   vec2 rel_pos = gl_FragCoord.xy * 0.05;

   const float margin = 0.05;
   float relprog = smoothstep( -margin, margin, pos.x-progress);

   const float time  = 0.0;
   const float hue   = 0.65;
   float value       = 0.4*(1.0-relprog);
   float brightness  = 0.1*relprog;

   color_out = nebula( vec4(0.0, 0.0, 0.0, 1.0), rel_pos, time, hue, value, brightness );
   color_out *= 1.0 - 0.8 * relprog;

   const float b = 8.0;
   float dist = sdBox( (pos.xy*2.0-1.0)*dimensions, vec2(dimensions.x-2.0*b,dimensions.y-2.0*b) );
   dist =1.0 - dist / b * 0.5;
   color_out.a *= dist;
}
