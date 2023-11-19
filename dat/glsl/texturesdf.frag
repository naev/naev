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
   float d  = (2.0*texture(sampler, tex_coord).r-1.0)*m;
   float alpha = smoothstep( -0.5, 0.5, d );
   //float beta  = smoothstep( -0.5, 0.5, d );
   float beta = alpha;
   color_out.rgb = mix( OUTLINE_COLOUR.rgb, color.rgb, beta );
   color_out.a = color.a*alpha;
}
