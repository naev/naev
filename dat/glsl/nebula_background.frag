#include "lib/nebula.glsl"

uniform float hue;
uniform float brightness;
uniform mat4 projection;
uniform float eddy_scale;
uniform float time;
out vec4 color_out;

void main(void) {
   vec2 rel_pos = gl_FragCoord.xy + projection[3].xy;
   rel_pos /= eddy_scale;
   color_out = nebula( vec4(0.0, 0.0, 0.0, 1.0), rel_pos, time, hue, 1.0, 0.1 );
   color_out.rgb *= brightness;
}
