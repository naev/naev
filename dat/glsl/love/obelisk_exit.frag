#include "lib/blur.glsl"

uniform float u_time = 0.0;
uniform float u_progress = 0.0;
uniform int u_invert = 0;

const float LENGTH = 3.0;
const vec4 FADECOLOUR = vec4( 0.0, 0.0, 0.0, 1.0 );

vec4 effect( sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
   float t = u_time / LENGTH;
   vec2 uv = texture_coords-0.5;
   float blur = max(1.0, 5.0-2.0*u_time );
   vec4 colour;
   if (blur <= 1.0)
      colour = texture(tex, texture_coords );
   else
      colour = blur5( tex, texture_coords, love_ScreenSize.xy, blur );

   return mix( FADECOLOUR, colour, min(1.0,u_time*2.0) );
}
