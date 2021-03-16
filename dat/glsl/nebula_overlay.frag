#include "lib/perlin.glsl"
#include "lib/colour.glsl"

const float SCALAR = pow(2., 4./3.);

uniform vec4 color;
uniform mat4 projection;
uniform float horizon;
uniform float eddy_scale;
uniform float time;
out vec4 color_out;

void main(void) {
   float dist, f;
   vec3 uv, rgb;

   // Compute coordinates for the noise
   vec2 rel_pos = gl_FragCoord.xy + projection[3].xy;
   uv.xy = 3. * rel_pos / eddy_scale + 1000.; // Scaled/offset from nebula_background
   uv.z = 1.5 * time;
   float hue = 240.0/360.0;

   // Do very simple two iteration noise
   f = abs( cnoise( uv * pow(SCALAR, 0) ) );
   f += abs( cnoise( uv * pow(SCALAR, 1) ) );
   rgb = hsv2rgb( vec3( hue, 1.0, 1.0 ) );
   color_out = vec4(rgb,1.0) * (0.1+f);

   // Compute dist and interpolate
   dist = length(rel_pos);
   color_out = mix( color_out, color, smoothstep( 0, 2*horizon, dist ) );
   color_out.a *= smoothstep( 0, horizon, dist );

#include "colorblind.glsl"
}
