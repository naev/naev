#include "lib/simplex.glsl"

uniform float u_time;
uniform vec3 u_camera; /* xy corresponds to screen space */

const vec3 COL_BACK  = vec3( 0.35, 0.3, 1.0 );
const vec3 COL_FRONT = vec3( 1.5, 0.65, 0.9 );
const float SCALE    = 200.0;
const float TIME_SCALE = 30.0;

vec3 hash( vec3 x )
{
   x = vec3( dot(x,vec3(127.1,311.7, 74.7)),
             dot(x,vec3(269.5,183.3,246.1)),
             dot(x,vec3(113.5,271.9,124.6)));
   return fract(sin(x)*43758.5453123);
}

/* 3D Voronoi- Inigo Qquilez (MIT) */
float voronoi( vec3 p )
{
   vec3 b, r, g = floor(p);
   p = fract(p);
   float d = 1.0;
   for(int j = -1; j <= 1; j++) {
      for(int i = -1; i <= 1; i++) {
         b = vec3(i, j, -1);
         r = b - p + hash(g+b);
         d = min(d, dot(r,r));
         b.z = 0.0;
         r = b - p + hash(g+b);
         d = min(d, dot(r,r));
         b.z = 1.0;
         r = b - p + hash(g+b);
         d = min(d, dot(r,r));
      }
   }
   return d;
}

/* Fractal brownian motion with voronoi! */
float voronoi_fbm( in vec3 p )
{
   float t = 0.0;
   float amp = 1.0;
   for (int i=0; i<5; i++) {
      t    += voronoi( p ) * amp;
      p    *= 2.0;
      amp  *= 0.5;
   }
   return t / 2.0;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec3 uv  = vec3( screen_coords / 150.0, u_time / 30.0 );
   uv.xy = ((texture_coords - 0.5) * love_ScreenSize.xy * u_camera.z + u_camera.xy) / SCALE;
   uv.z = u_time / TIME_SCALE;

   float vf = voronoi_fbm( uv );
   float v  = 4.0*vf*vf*vf;

   vec4 colour_out;
   colour_out.rgb = mix( COL_BACK, COL_FRONT, v );
   colour_out.a = v;

   float flash_0 = sin(u_time);
   float flash_1 = sin(15.0 * u_time);
   float flash_2 = sin(2.85 * u_time);
   float flash_3 = sin(5.18 * u_time);
   float flash = max( 0.0,  snoise( uv*0.5 ) * flash_0 * flash_1 * flash_2 * flash_3 );
   if (flash > 0.0) {
      float ff;
      ff = max(vf + dot(hash(uv)*2.0 - 1.0, vec3(0.006)), 0.0);
      ff = pow(ff*1.55,2.5);
      colour_out += 1.5 * flash * ff;
   }

   return colour_out;
}
