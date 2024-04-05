#include "lib/math.glsl"

uniform float u_time = 0.0;
uniform vec3 u_camera = vec3( 0.0, 0.0, 1.0 );

const float strength = %f;
const float speed    = %f;
const float u_r      = %f;
const int  NUM_OCTAVES = 3;

/* 1D noise */
float noise1( float p )
{
   float fl = floor(p);
   float fc = fract(p);
   return mix(random(fl), random(fl + 1.0), fc);
}

/* Voronoi distance noise. */
float voronoi( vec2 x )
{
   vec2 p = floor(x);
   vec2 f = fract(x);

   vec2 res = vec2(8.0);
   for(int j = -1; j <= 1; j++) {
      for(int i = -1; i <= 1; i++) {
         vec2 b = vec2(i, j);
         vec2 r = vec2(b) - f + random(p + b);

         /* Chebyshev distance, one of many ways to do this */
         float d = max(abs(r.x), abs(r.y));

         if (d < res.x) {
            res.y = res.x;
            res.x = d;
         }
         else if (d < res.y)
            res.y = d;
      }
   }
   return res.y - res.x;
}

vec4 effect( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   /* Calculate coordinates relative to camera. */
   vec2 uv = (texture_coords - 0.5) * love_ScreenSize.xy * u_camera.z + u_camera.xy + u_r;
   uv *= strength / 500.0; /* Normalize so that strength==1.0 looks fairly good. */

   /* Minor flickering between 0.9 and 1.1. */
   float flicker = noise1( u_time * 2.0 * speed ) * 0.2 + 0.9;

   /* Add some noise octaves */
   float a = 0.6;
   float f = 1.0;

   /* 4 octaves also look nice, its getting a bit slow though */
   float v = 0.0;
   for (int i=0; i < NUM_OCTAVES; i ++) {
      float v1 = voronoi(uv * f + 5.0);
      float v2 = 0.0;

      /* Make the moving electrons-effect for higher octaves. */
      if (i > 0) {
         v2 = voronoi(uv * f * 0.5 + 50.0 + u_time * speed);

         float va = 1.0 - smoothstep(0.0, 0.1,  v1);
         float vb = 1.0 - smoothstep(0.0, 0.08, v2);
         v += a * pow(va * (0.5 + vb), 2.0);
      }

      /* Sharpen the edges. */
      v1 = 1.0 - smoothstep(0.0, 0.3, v1);

      /* Noise is used as intensity map */
      v2 = a * (noise1(v1 * 5.5 + 0.1));

      /* Octave 0's intensity changes a bit */
      if (i == 0)
         v += v2 * flicker;
      else
         v += v2;

      f *= 3.0;
      a *= 0.7;
   }

   /* Blueish colour set */
   vec3 cexp = vec3(6.0, 4.0, 2.0);

   /* Convert to colour, clamp and multiply by base colour. */
   vec3 col = vec3(pow(v, cexp.x), pow(v, cexp.y), pow(v, cexp.z)) * 2.0;
   return colour * vec4( clamp( col, 0.0, 1.0 ), 1.0);
}
