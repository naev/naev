const int ITERATIONS = 3;
const float SCALAR = pow(2., 4./3.);

uniform mat4 projection;
uniform float eddy_scale = 100.0;
uniform float u_time = 0.0;

vec4 effect( vec4 col_in, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec3 uv;

   // Calculate coordinates
   uv.xy = screen_coords / eddy_scale;
   uv.z = u_time / 5.0;

   // Create the noise
   float f;
   f  = (1.0-cellular( uv     ).x) * 0.625;
   f += (1.0-cellular( uv*2.0 ).x) * 0.25;
   f += (1.0-cellular( uv*4.0 ).x) * 0.125;

   vec4 colour =  mix( vec4(0.0,0.0,0.0,1.0), col_in, f );

   return colour;
}
