#include "lib/sdf.glsl"

uniform float u_progress = 0.0;

const vec3 FADECOL = vec3( 0.6, 0.6, 1.0 );

vec4 effect( vec4 color, Image tex, vec2 uv, vec2 px )
{
   vec4 col = texture( tex, uv );
   col.rgb = mix( vec3(0.6, 0.2, 0.8)*0.2, vec3(0.0), u_progress );
   col.a *= u_progress * 0.35;
   return col;
}
