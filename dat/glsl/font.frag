uniform vec4 color;
uniform sampler2D sampler;

in vec2 tex_coord_out;
out vec4 color_out;

const float glyph_center   = 0.5;

void main(void) {
   /* Standard rendering. */
   /*
   color_out = color;
   color_out.a = texture(sampler, tex_coord_out).r;
   */

   /* Distance field rendering. */
   float dist = texture(sampler, tex_coord_out).r;
   float width = fwidth(dist);
   float alpha = smoothstep(glyph_center-width, glyph_center+width, dist);
   color_out = vec4(color.rgb, alpha*color.a);

#include "colorblind.glsl"
}
