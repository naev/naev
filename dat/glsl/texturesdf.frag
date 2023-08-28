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
   float mm = 0.1;
   float d  = 1.0 - 2.0*texture(sampler, tex_coord).r;
   float alpha = smoothstep( 1.0*mm, 2.0*mm, d);
   float beta  = smoothstep( 0.0*mm, 1.0*mm, d);
   vec4 fg_c   = mix( OUTLINE_COLOUR, color, alpha );
   color_out   = vec4( fg_c.rgb, beta*fg_c.a );
}
