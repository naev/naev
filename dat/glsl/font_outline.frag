uniform vec4 color;
uniform vec4 outline_color;
uniform sampler2D sampler;

in vec2 tex_coord_out;
out vec4 color_out;

// How thick to render glyphs; 0 is very thick, 1 is very thin
const float glyph_center   = 0.5;
// between 0 and glyph_center is a thick outline, above glyph_center is no outline
const float outline_center = 0.1;

void main(void) {
   // dist is a value between 0 and 1 with 0.5 on the edge and 1 inside it.
   float dist = texture(sampler, tex_coord_out).r;
   // fwidth computes the absolute value of the x and y derivatives
   float width = fwidth(dist)/2;
   // smoothstep maps values below 0.5 to 0 and above 0.5 to 1, with a smooth transition at 0.5.
   float alpha = smoothstep(glyph_center-width, glyph_center+width, dist);
   float beta = smoothstep(outline_center-width, outline_center+width, dist);
   vec3 rgb = mix(outline_color.rgb, color.rgb, alpha);
   color_out = vec4(rgb, beta*color.a);

#include "colorblind.glsl"
}
