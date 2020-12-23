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
   // Average and half-difference of the absolute x- and y-derivatives.
   float sigma = fwidth(dist) / 2;
   float delta = abs(abs(dFdx(dist))-abs(dFdy(dist))) / 2;
   float alpha = clamp(.5 + (dist-glyph_center + clamp(dist-glyph_center,-delta,delta))/(sigma+delta+1e-4), 0, 1);
   float beta = clamp(.5 + (dist-outline_center + clamp(dist-outline_center,-delta,delta))/(sigma+delta+1e-4), 0, 1);
   vec3 rgb = mix(outline_color.rgb, color.rgb, alpha);
   color_out = vec4(rgb, beta*color.a);

#include "colorblind.glsl"
}
