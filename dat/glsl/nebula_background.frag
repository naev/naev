#include "lib/nebula.glsl"
#
uniform vec4 color;
uniform mat4 projection;
uniform float eddy_scale;
uniform float time;
out vec4 color_out;


void main(void) {
   vec2 rel_pos = gl_FragCoord.xy + projection[3].xy;
   rel_pos /= eddy_scale;
   float hue = 240.0/360.0;
   color_out = nebula( vec4(0.0, 0.0, 0.0, 1.0), rel_pos, time, hue, 0.1 );

#include "colorblind.glsl"
}
