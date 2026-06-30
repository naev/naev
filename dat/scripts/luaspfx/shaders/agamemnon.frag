#include "lib/simplex.glsl"

uniform float u_time;
uniform float u_r;

vec4 effect(vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords)
{
   vec2 p = 2.0*texture_coords - 1.0;

   float r = length(p);
   vec2 dir = normalize(p + 1e-6);
   float t = u_time + 100.0*u_r;

   float a = snoise(dir * 1.0 + vec2(t * 0.15))
            + 0.5 * snoise(dir * 2.0 - vec2(t * 0.30));
   a /= 1.5;

   float radius = 0.2 +
      // Bleeding light
      (0.2 * a + 0.35 * pow(a,2.0)) * smoothstep( 1.0, 0.0, max( 0.0, (u_time-1.6)*3.0 ) );

   // Make it pop in and out
   radius *= smoothstep( -0.2, 1.0, min( 1.0, u_time*0.5 ) )
           * smoothstep( 1.0, -0.2, max( 0.0, (u_time-1.9)*10.0) );

   // Some glow
   float glow = exp(-5.0 * max(r - radius, 0.0));

   // Colour is hardcoded mainly
   vec3 col = vec3(1.4, 0.5, 1.3) * glow;
   float alpha = glow
               * smoothstep( 0.0, 1.0, min( 1.0, u_time*2.0 ) )
               * smoothstep( 1.0, 0.0, max( 0.0, (u_time-1.9)*10.0) );

   return vec4(col, alpha) * color;
}
