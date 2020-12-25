uniform vec4 color;
uniform sampler2D sampler;

in vec2 tex_coord_out;
out vec4 color_out;

// How thick to render glyphs; 0 is very thick, 1 is very thin
const float glyph_center   = 0.5;

void main(void) {
   /* Standard rendering. */
   /*
   color_out = color;
   color_out.a = texture(sampler, tex_coord_out).r;
   */

   /* Distance field rendering. */
   // dist is a value between 0 and 1 with 0.5 on the edge and 1 inside it.
   float dist = texture(sampler, tex_coord_out).r;
   // fwidth sums the absolute value of the x and y derivatives
   float hw = fwidth(dist)/2;
   // smoothstep maps values below 0.5 to 0 and above 0.5 to 1, with a smooth transition at 0.5.
   float alpha = smoothstep(glyph_center-hw, glyph_center+hw, dist);
   color_out = vec4(color.rgb, alpha*color.a);

#include "colorblind.glsl"
}
