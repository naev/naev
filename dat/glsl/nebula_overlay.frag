#include "lib/perlin.glsl"
#include "lib/colour.glsl"
#include "lib/nebula.glsl"

uniform float hue;
uniform mat4 projection;
uniform float horizon;
uniform float eddy_scale;
uniform float time;
out vec4 color_out;

void main(void) {
   float dist, f, hhue;
   vec3 uv;
   vec4 color;
   vec2 rel_pos;

   /* Compute coordinates for the noise */
   rel_pos = gl_FragCoord.xy + projection[3].xy;
   uv.xy = rel_pos / eddy_scale;
   uv.z = time;

   /* Compute hue as in lib/nebula.glsl. */
   hhue = nebula_hue( hue, uv );

   /* Modify coordinates to be larger and slower. */
   uv.xy = 3. * uv.xy + 1000.; // Scaled/offset from nebula_background
   uv.z *= 1.5;

   /* Do very simple two iteration noise */
   f = abs( cnoise( uv * pow(SCALAR, 0) ) );
   f += abs( cnoise( uv * pow(SCALAR, 1) ) );
   color = vec4( hsv2rgb( vec3( hhue, 1.0, 1.0 ) ), 1.0 );
   color_out = color * (0.1+0.9*f);

   /* Compute dist and interpolate */
   dist = length(rel_pos);
   color_out = mix( color_out, color, smoothstep( 0, 2*horizon, dist ) );
   color_out.a *= smoothstep( 0, horizon, dist );
}
