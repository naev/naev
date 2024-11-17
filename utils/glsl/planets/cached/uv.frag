
uniform vec2 dimensions;

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 color_out = vec4(1.0);
   // Scaled UV coordinates.
   vec2 uv = screen_coords / dimensions;
   color_out.rg = uv;

   return color_out;
}
