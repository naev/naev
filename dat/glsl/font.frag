uniform vec4 color;
uniform sampler2D sampler;

in vec2 tex_coord_out;
out vec4 color_out;

const float glyph_center   = 0.5;
const vec3  outline_color  = vec3( 0., 0., 0. );
const float outline_center = 0.55;

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
   float beta = smoothstep(outline_center-width, outline_center+width, dist);
   vec3 rgb = mix(outline_color, color.rgb, beta);
   color_out = vec4(rgb, alpha*color.a);
   //color_out = vec4(color.rgb, alpha*color.a);

#include "colorblind.glsl"
}
