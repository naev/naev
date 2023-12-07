#include "lib/simplex.glsl"
#include "lib/wavenoise.glsl"
#include "lib/colour.glsl"

uniform float u_time;
uniform vec3 u_camera; /* xy corresponds to screen space */

const float DRAGMULT    = 10.0;
const float SCALE       = 300.0;
const vec3 COL_BACK     = vec3( 150.0, 50.0, 150.0 ) / 255.;
const vec3 COL_FRONT    = vec3( 220.0, 80.0, 220.0 ) / 255.;

vec4 effect( vec4 colour_in, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec3 uv;
   uv.xy = ((texture_coords - 0.5) * love_ScreenSize.xy * u_camera.z + u_camera.xy) / SCALE;
   uv.z = u_time;

   /* Generate the base noise. */
   float s = wavenoise( uv.xy, DRAGMULT, snoise(vec3(uv.xy*1.7,uv.z/10.0))+u_time*0.1);
   s = s*s;
   vec4 colour;
   colour.rgb = mix( COL_BACK, COL_FRONT, s );
   colour.a = s;

   /* Flashing parts. */
   float flash_0 = sin(u_time);
   float flash_1 = sin(15.0 * u_time);
   float flash_2 = sin(2.85 * u_time);
   float flash_3 = sin(5.18 * u_time);
   float flash = max( 0.0,  snoise( uv*0.5 ) * flash_0 * flash_1 * flash_2 * flash_3 );
   colour += vec4( 1.5*(s+0.5*flash)*flash*(0.5+0.5*flash_2) );
   colour = clamp( colour, vec4(0.0), vec4(1.0) );

   /* Correct and out it goes. */
   return colour * colour_in;
}
