#include "lib/simplex.glsl"

uniform float u_time = 0.0;
uniform sampler2D u_bgtex;

const vec3 col_inner  = vec3( %f, %f, %f );
const vec3 col_outter = vec3( %f, %f, %f );

float fbm3( vec3 x )
{
   float v = 0.0;
   float a = 0.5;
   const vec3 shift = vec3(100.0);
   for (int i=0; i<3; i++) {
      v += a * snoise(x);
      x  = x * 2.0 + shift;
      a *= 0.5;
   }
   return v;
}

float fbm5( vec3 x )
{
   float v = 0.0;
   float a = 0.5;
   const vec3 shift = vec3(100.0);
   for (int i=0; i<5; i++) {
      v += a * snoise(x);
      x  = x * 2.0 + shift;
      a *= 0.5;
   }
   return v;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   float t = u_time * 0.2;

   vec2 uv = texture_coords-0.5;
   vec2 st = vec2( 1.5 * length(uv), atan(uv.y,uv.x) );

   st.y += 1.1 * st.x;
   float x = fbm3( vec3( sin(st.y), cos(st.y), pow(st.x, 0.3) + 0.1*t ) ); // 3
   float y = fbm3( vec3( cos(1.0-st.y), sin(1.0-st.y), pow(st.x, 0.5) + 0.1*t ) ); // 4

   float r = fbm5( vec3( x, y, st.x + 0.3*t ) ); // 5
   r = fbm5( vec3( r-x, r-y, r + 0.3*t ) ); // 6

   float c = (r + 5.0*st.x) / 6.0;

   /* Determine the inner and outter alpha values. */
   float a_in  = 1.0-smoothstep( 0.2, 0.35, c );
   float a_out = 1.0-smoothstep( 0.35, 0.6, c );

   /* Set up the colours. */
   vec3 col = vec3(0.0);
   col = mix( col, col_outter, a_out );
   col = mix( col, col_inner,  a_in );

   /* Inner stuff. */
   vec4 bg = texture( u_bgtex, texture_coords );
   col = mix( col, bg.rgb, a_in-0.03 );

   return vec4( col, a_out );
}
