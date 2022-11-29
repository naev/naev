#include "lib/sdf.glsl"

uniform vec2 u_dimensions = vec2(0.0);

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   uv             = (uv * 2.0 - 1.0) * u_dimensions;
   float d        = sdTrapezoid( uv.yx, u_dimensions.y*0.8, u_dimensions.y, u_dimensions.x );
   float alpha    = smoothstep( 0.0, 10.0, -d);
   vec4 colour_out = colour;
   colour_out.a   *= alpha;
   return colour_out;
}
