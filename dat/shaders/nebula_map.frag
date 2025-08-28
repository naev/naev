#include "lib/nebula.glsl"

uniform float hue;
uniform float brightness = 1.0;
uniform float time;
uniform vec2 globalpos;
uniform float alpha;
uniform float volatility = 0.0;
in vec2 localpos;
out vec4 colour_out;

const float smoothness       = 0.5;
const float value            = 0.6;
const float nebu_brightness  = 0.4;

void main (void)
{
   /* Fallout */
   float dist = length(localpos);
   dist = (dist < 1.0-smoothness) ? 1.0 : (1.0 - dist) / smoothness;
   float a = smoothstep( 0.0, 1.0, dist );
   if (a <= 0.0) {
      discard;
      return;
   }

   /* Calculate coordinates */
   vec2 rel_pos = localpos + globalpos;
   colour_out = nebula( vec4(0.0), rel_pos, time, hue, value, volatility, nebu_brightness );
   colour_out.a *= a * alpha;
   if (brightness < 1.0) {
      vec4 base = vec4( hsv2rgb( vec3(hue, value, 1.0) ), 1.0 );
      colour_out = mix( base, colour_out, brightness );
   }
}
