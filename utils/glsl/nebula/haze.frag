const int ITERATIONS = 3;
const float SCALAR = 2.0; // pow(2., 4./3.);

uniform vec4 color;
uniform mat4 projection;
uniform float eddy_scale = 200.0;
uniform float u_time = 0.0;
uniform vec2 center = vec2(400, 225);
out vec4 color_out;

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   float f = 0.0;
   vec3 uv;

   // Calculate coordinates
   uv.xy = screen_coords / eddy_scale;
   uv.z = u_time / 20.0;

   // Create the noise
   for (int i=0; i<ITERATIONS; i++) {
      float scale = pow(SCALAR, i);
      f += (snoise( uv * scale )*0.5 + 0.2) / scale;
   }

   float d = min( 1.0, length( screen_coords - center ) / 200.0 );

   return mix( vec4(0.0,0.0,0.0,1.0), color, f*d );
}
