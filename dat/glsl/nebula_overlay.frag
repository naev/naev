#include "lib/perlin3D.glsl"

const float SCALAR = pow(2., 4./3.);

uniform vec4 color;
uniform vec2 center;
uniform float radius;
uniform float time;
out vec4 color_out;

void main(void) {
   float dist, f;
   vec3 uv;

   // Compute coordinates for the noise
   // Offset with respect to nebula_background
   uv.xy = 0.1 * (gl_FragCoord.xy-center) / pow(radius, 0.7) + 1000.;
   uv.z = time * 0.7; // Slower than background

   // Do very simple two iteration noise
   f = abs( cnoise( uv * pow(SCALAR, 1) ) );
   f += abs( cnoise( uv * pow(SCALAR, 2) ) );
   color_out = color * (.1+f);

   // Compute dist and interpolate
   dist = distance(gl_FragCoord.xy, center);
   color_out = mix( color_out, color, smoothstep( 0, 2*radius, dist ) );
   color_out.a *= smoothstep( 0, radius, dist );

#include "colorblind.glsl"
}
