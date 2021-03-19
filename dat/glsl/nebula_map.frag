#include "lib/nebula.glsl"

uniform float hue;
uniform mat4 projection;
uniform float eddy_scale;
uniform float time;
uniform vec2 globalpos;
in vec2 localpos;
out vec4 color_out;

const float smoothness  = 0.5;
const float value       = 0.4;
const float brightness  = 0.5;

void main(void) {
   float f = 0.0;
   vec3 uv;

   // Calculate coordinates
   vec2 rel_pos = gl_FragCoord.xy + projection[3].xy - globalpos;
   rel_pos /= eddy_scale;
   color_out = nebula( vec4(0.0), rel_pos, time, hue, value, brightness );

   // Fallout
   float dist = length(localpos);
   dist = (dist < 1.0-smoothness) ? 1.0 : (1.0 - dist) / smoothness;
   color_out.a *= smoothstep( 0.0, 1.0, dist );
}
