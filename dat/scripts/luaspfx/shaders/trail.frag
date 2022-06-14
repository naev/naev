#include "lib/perlin.glsl"

const int ITERATIONS = 2;
const float SCALAR = pow(2., 4./3.);
const float eddy_scale = 50.0;

uniform float u_time = 0.0;
uniform float u_size = 0.0;
uniform float u_r = 0.0;
uniform vec2 u_vel = vec2(0.0);

float fbm( vec4 color, vec2 screen_coords )
{
   float f = 0.0;
   vec3 uv;

   // Calculate coordinates
   uv.xy = screen_coords / eddy_scale + u_r;
   uv.z = u_time / 15.0;

   // Create the noise
   for (int i=0; i<ITERATIONS; i++) {
      float scale = pow(SCALAR, i);
      f += cnoise( uv * scale ) / scale;
   }
	return 0.1 + f;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec2 uv = (texture_coords*2.0-1.0);
   float d = length(uv);
	float n = fbm( color, (uv + 0.3 * u_time * u_vel) * u_size * 0.5 );
   color.a *= n * smoothstep( -1.0, 0.0, -d ) * min( 1.0, u_time );
   return color;
}
