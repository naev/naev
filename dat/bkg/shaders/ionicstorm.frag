#include "lib/math.glsl"
#include "lib/simplex.glsl"

/*
 * Noise function modified from https://www.shadertoy.com/view/lscyD7
 * afl_ext 2018
 * public domain
 */

uniform float u_time;
uniform vec3 u_camera = vec3(1.0); /* xy corresponds to screen space */

const int ITERATIONS    = 50;
const float DRAGMULT    = 10.0;
const float SCALE       = 500.0;

float wave( vec2 uv, vec2 emitter, float speed, float phase, float timeshift )
{
   float dst = distance(uv, emitter);
   return pow(M_E, sin(dst * phase - timeshift * speed)) / M_E;
}
vec2 wavedrag( vec2 uv, vec2 emitter )
{
   return normalize(uv - emitter);
}
float seedWaves = 0.0;
vec2 randWaves2(void)
{
   float x = random(seedWaves);
   seedWaves += 1.0;
   float y = random(seedWaves);
   seedWaves += 1.0;
   return vec2(x,y) * 2.0 - 1.0;
}
float getwaves3d( vec2 position, float dragmult, float timeshift )
{
   float iter  = 0.0;
   float phase = 6.0;
   float speed = 2.0;
   float weight= 1.0;
   float w     = 0.0;
   float ws    = 0.0;
   for(int i=0;i<ITERATIONS;i++) {
      vec2 p = randWaves2() * 30.0;
      float res  = wave(position, p, speed, phase, 0.0 + timeshift);
      float res2 = wave(position, p, speed, phase, 0.006 + timeshift);
      position -= wavedrag(position, p) * (res - res2) * weight * dragmult;
      w    += res * weight;
      iter += 12.0;
      ws   += weight;
      weight= mix(weight, 0.0, 0.2);
      phase*= 1.2;
      speed*= 1.02;
   }
   return w / ws;
}

vec4 effect( vec4 colour_in, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec3 uv;
   uv.xy = ((texture_coords - 0.5) * love_ScreenSize.xy * u_camera.z + u_camera.xy) / SCALE;
   uv.z = u_time;

   /* Generate the base noise. */
   float s = getwaves3d( uv.xy, DRAGMULT, u_time*0.3);
   s = s*s;
   vec4 colour = vec4( vec3(1.0), s );

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
