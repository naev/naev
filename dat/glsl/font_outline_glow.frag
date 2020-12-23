uniform vec4 color;
uniform vec4 outline_color;
uniform vec4 glow_color;
uniform sampler2D sampler;

in vec2 tex_coord_out;
out vec4 color_out;

const float glyph_center   = 0.5;
const float outline_center = 0.55;
const float glow_center    = 1.25;

void main(void) {
   float dist = texture(sampler, tex_coord_out).r;
   float width = fwidth(dist)/2;
   float alpha = smoothstep(glyph_center-width, glyph_center+width, dist);

   vec3 rgb = mix(glow_color.rgb, color.rgb, alpha);
   float mu = smoothstep(glyph_center, glow_center, sqrt(dist));
   color_out = vec4(rgb, max(alpha,mu));
   float beta = smoothstep(outline_center-width, outline_center+width, dist);
   rgb = mix(outline_color.rgb, color_out.rgb, beta);
   color_out = vec4(rgb, max(color.a, beta));

#include "colorblind.glsl"
}
