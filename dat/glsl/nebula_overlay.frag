#include "lib/perlin.glsl"
#include "lib/colour.glsl"
#include "lib/nebula.glsl"

uniform float hue;
uniform float brightness;
uniform mat4 projection;
uniform float horizon;
uniform float eddy_scale;
uniform float time;
in vec4 base_col;
out vec4 colour_out;

void main(void) {
   float dist, f, hhue;
   vec3 uv;
   vec4 colour;
   vec2 rel_pos;

   /* Compute coordinates for the noise */
   rel_pos = gl_FragCoord.xy + projection[3].xy;
   uv.xy = rel_pos / eddy_scale;
   uv.z = time * 0.5;

   /* Compute hue as in lib/nebula.glsl. */
   hhue = nebula_hue( hue, uv );
   colour = base_col;

   /* Modify coordinates to be larger and slower. */
   uv.xy = 3.0 * uv.xy + 1000.0; // Scaled/offset from nebula_background
   uv.z *= 1.5;

   /* Do very simple two iteration noise */
   if (brightness > 0.0) {
      f = abs( cnoise( uv * pow(SCALAR, 0.0) ) );
      f += abs( cnoise( uv * pow(SCALAR, 1.0) ) );
      colour_out = colour * (0.1+0.9*f);
   }
   else {
      colour_out = vec4(1.0);
   }

   if (brightness < 1.0) {
      vec4 base = base_col;
      colour_out = mix( base, colour_out, brightness );
   }

   /* Compute dist and interpolate */
   dist = length(rel_pos);
   colour_out = mix( colour_out, colour, smoothstep( 0.0, 2.0*horizon, dist ) );
   colour_out.a *= smoothstep( 0.0, horizon, dist );
}
