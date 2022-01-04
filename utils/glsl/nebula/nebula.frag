const int ITERATIONS = 3;
const float SCALAR = pow(2., 4./3.);

uniform mat4 projection;
uniform float eddy_scale = 50.0;
uniform float u_time = 0.0;

vec4 effect( vec4 col_in, Image tex, vec2 texture_coords, vec2 screen_coords )
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

   vec4 colour =  mix( vec4(0.0,0.0,0.0,1.0), col_in, 0.1 + f );

   // Some volatility
   float flash_0 = sin(u_time);
   float flash_1 = sin(15.0 * u_time);
   float flash_2 = sin(2.85 * u_time);
   float flash_3 = sin(5.18 * u_time);
   uv.z  *= 30.0; /* Increae time. */
   float flash = max( 0.0,  snoise( uv*0.1 ) * flash_0 * flash_1 * flash_2 * flash_3 );
   colour.rgb += vec3( 2.0*(f+0.5*flash)*flash*(0.5+0.5*flash_2) );

   return colour;
}
