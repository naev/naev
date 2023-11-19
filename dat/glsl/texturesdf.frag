#include "lib/math.glsl"

uniform vec4 color;
uniform sampler2D sampler;
uniform float m;
uniform float outline;

in vec2 tex_coord;
out vec4 color_out;

const vec4 OUTLINE_COLOUR = vec4( vec3(0.0), 1.0 );

void main(void)
{
   float d  = (texture(sampler, tex_coord).r-0.5)*m;
   float alpha = smoothstep(-0.5    , +0.5, d);
   float beta  = smoothstep(-M_SQRT2, -1.0, d);
   vec4 fg_c   = mix( OUTLINE_COLOUR, color, alpha );
   color_out   = vec4( fg_c.rgb, beta*fg_c.a );
}
