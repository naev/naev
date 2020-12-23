
uniform vec4 color;
uniform sampler2D sampler;

in vec2 tex_coord_out;
out vec4 color_out;

const float glyph_center   = 0.5;

void main(void) {
   float dist = texture(sampler, tex_coord_out).r;
   float sigma = abs(abs(dFdx(dist))+abs(dFdy(dist)))/2;
   float delta = abs(abs(dFdx(dist))-abs(dFdy(dist)))/2;
   float alpha = clamp(.5 + (dist-glyph_center + clamp(dist-glyph_center,-delta,delta))/(sigma+delta+1e-4), 0, 1);
   color_out = vec4(color.rgb, alpha*color.a);

#include "colorblind.glsl"
}
