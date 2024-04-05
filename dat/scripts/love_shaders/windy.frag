#include "lib/simplex.glsl"

uniform float u_time = 0.0;
uniform vec3 u_camera = vec3( 0.0, 0.0, 1.0 );

const float strength = %f;
const float speed    = %f;
const float density  = %f;
const float u_r      = %f;

const float noiseScale = 0.001;
const float noiseTimeScale = 0.03;

float fbm3(vec3 v) {
   float result = snoise(v);
   result += snoise(v * 2.0) / 2.0;
   result += snoise(v * 4.0) / 4.0;
   result /= (1.0 + 1.0/2.0 + 1.0/4.0);
   return result;
}

float getNoise(vec3 v) {
   v.xy += vec2( fbm3(v), fbm3(vec3(v.xy, v.z + 1000.0)));
   return fbm3(v) / 2.0 + 0.5;
}

vec4 effect( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   float f = 0.0;
   vec3 uv;

   /* Calculate coordinates */
   uv.xy = (texture_coords - 0.5) * love_ScreenSize.xy * u_camera.z + u_camera.xy + u_r;
   uv.xy *= strength;
   uv.z  = u_time * speed;

   uv *= vec3( noiseScale, noiseScale, noiseTimeScale );

   float noise = getNoise( uv );
   noise = pow( noise, 4.0 / density ) * 2.0;  //more contrast
   return colour * vec4( 1.0, 1.0, 1.0, noise );
}
