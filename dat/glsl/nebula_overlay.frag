#include "lib/perlin3D.glsl"

const float SCALAR = pow(2., 4./3.);

uniform vec4 color;
uniform mat4 projection;
uniform float radius;
uniform float time;
out vec4 color_out;

void main(void) {
   float dist, f;
   vec3 uv;

   // Compute coordinates for the noise
   vec2 rel_pos = gl_FragCoord.xy + projection[3].xy;
   uv.xy = rel_pos / radius + 1000.; // Offset from nebula_background
   uv.z = time * 0.7; // Slower than background

   // Do very simple two iteration noise
   f = abs( cnoise( uv * pow(SCALAR, 1) ) );
   f += abs( cnoise( uv * pow(SCALAR, 2) ) );
   color_out = color * (.1+f);

   // Compute dist and interpolate
   dist = length(rel_pos);
   color_out = mix( color_out, color, smoothstep( 0, 2*radius, dist ) );
   color_out.a *= smoothstep( 0, radius, dist );

#include "colorblind.glsl"
}
