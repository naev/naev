#include "lib/nebula.glsl"

uniform float hue;
uniform float brightness = 1.0;
uniform float eddy_scale;
uniform float time;
uniform vec2 globalpos;
uniform float alpha;
in vec2 localpos;
out vec4 color_out;

const float smoothness       = 0.5;
const float value            = 0.4;
const float nebu_brightness  = 0.5;

void main(void) {
   float f = 0.0;
   vec3 uv;

   // Calculate coordinates
   vec2 rel_pos = localpos + globalpos;
   //rel_pos *= eddy_scale;
   color_out = nebula( vec4(0.0), rel_pos, time, hue, value, nebu_brightness );
   color_out.rgb *= brightness;

   // Fallout
   float dist = length(localpos);
   dist = (dist < 1.0-smoothness) ? 1.0 : (1.0 - dist) / smoothness;
   color_out.a *= smoothstep( 0.0, 1.0, dist ) * alpha;
}
