uniform vec4 color;
uniform vec4 outline_color;
uniform sampler2D sampler;

in vec2 tex_coord_out;
out vec4 color_out;

// Colour cutoffs, corresponding to "dist" below.
const float glyph_center     = 0.5;
const float outline_center   = 0.2;
const float glyph_stepsize   = 0.1;
const float outline_stepsize = 0.125;

void main(void) {
   // dist is a value between 0 and 1 with 0.5 on the edge and 1 inside it.
   float dist = texture(sampler, tex_coord_out).r;
   // smoothstep maps values below 0.5 to 0 and above 0.5 to 1, with a smooth transition at 0.5.
   float alpha = smoothstep(glyph_center-glyph_stepsize, glyph_center+glyph_stepsize, dist);
   float beta = smoothstep(outline_center-outline_stepsize, outline_center+outline_stepsize, dist);
   vec4 fg_c = mix(outline_color, color, alpha);
   color_out = vec4(fg_c.rgb, beta*fg_c.a);

#include "colorblind.glsl"
}
