const int ITERATIONS = 3;
const float SCALAR = pow(2., 4./3.);

uniform vec4 color;
uniform mat4 projection;
uniform float eddy_scale = 50.0;
uniform float u_time = 0.0;
out vec4 color_out;

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   float f = 0.0;
   vec3 uv;

   // Calculate coordinates
   uv.xy = screen_coords / eddy_scale;
   uv.z = u_time / 5.0;

   // Create the noise
   for (int i=0; i<ITERATIONS; i++) {
      float scale = pow(SCALAR, i);
      f += abs( cnoise( uv * scale ) ) / scale;
   }
   return mix( vec4(0.0,0.0,0.0,1.0), color, .1 + f );
}
