#include "lib/blur.glsl"

uniform float u_time = 0.0;

const float LENGTH = 3.0;
const vec4 FADECOLOUR = vec4( 0.0, 0.0, 0.0, 1.0 );

vec4 effect( sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
   vec2 uv = texture_coords-0.5;
   float blur = 1.0 + 4.0/LENGTH*(LENGTH-u_time);
   vec4 colour;
   if (blur <= 1.0)
      colour = texture(tex, texture_coords );
   else
      colour = blur9( tex, texture_coords, love_ScreenSize.xy, blur );

   return mix( FADECOLOUR, colour, min(1.0,2.0/LENGTH*u_time) );
}
