#include "lib/sdf.glsl"
#include "lib/blur.glsl"

uniform float u_progress = 0.0;

const vec4 FADECOLOUR = vec4( 0.0, 0.0, 0.0, 1.0 );

vec4 effect( sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
   vec2 uv = texture_coords-0.5;
   float blur = 5.0*2.0*max(0.0, u_progress-0.5);
   vec4 colour;
   if (blur <= 0.0)
      colour = texture( tex, texture_coords );
   else
      colour = blur5( tex, texture_coords, love_ScreenSize.xy, blur );

   float d = sdVesica( uv.yx, 0.8, u_progress );
   float a = (1.0-smoothstep( 0.0, 0.2, -d )) * min(1.0, u_progress*2.0);

   return mix( colour, FADECOLOUR, a );
}
