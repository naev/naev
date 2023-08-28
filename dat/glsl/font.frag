#include "lib/math.glsl"

uniform float m;
uniform vec4 color;
uniform vec4 outline_color;
uniform sampler2D sampler;

in vec2 tex_coord_out;
out vec4 color_out;

void main(void)
{
   // d is the signed distance to the glyph; m is the distance value corresponding to 1 "pixel".
   float d  = texture(sampler, tex_coord_out).r - 0.5;
   // Map the signed distance to mixing parameters for outline..foreground, transparent..opaque.
   float alpha = smoothstep(-0.5    *m, +0.5*m, d);
   float beta  = smoothstep(-M_SQRT2*m, -1.0*m, d);
   vec4 fg_c   = mix( outline_color, color, alpha );
   color_out   = vec4( fg_c.rgb, beta*fg_c.a );
   gl_FragDepth = -d*0.5 + 0.5;
}
