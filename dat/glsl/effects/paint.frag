#include "lib/blend.glsl"

uniform vec3 dimensions;
uniform sampler2D u_tex;

in vec2 tex_coord;
out vec4 colour_out;

const vec3 COLOUR    = vec3( 0.45, 0.001, 0.004 );

void main(void)
{
   colour_out = texture (u_tex, tex_coord);
   float gray = dot(colour_out.rgb, vec3(0.299, 0.587, 0.114));
   colour_out.rgb = blendSoftLight(vec3(gray, gray, gray), COLOUR );
}
