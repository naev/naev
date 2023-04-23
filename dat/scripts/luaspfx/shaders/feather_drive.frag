#include "lib/sdf.glsl"
#include "lib/simplex.glsl"

uniform float u_r = 0.0;
uniform float u_progress = 0.0;

const vec3 FADECOL = vec3( 1.0, 0.8, 0.0 );

vec4 effect( vec4 color, Image tex, vec2 uv, vec2 px )
{
   vec4 col = texture( tex, uv );
   float n = 0.5 + 0.3*snoise( 3.0*uv + vec2(u_r) );
   col.rgb += u_progress * FADECOL * n;
   col.a *= u_progress;
   return col;
}
