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
   float t = texture(sampler, tex_coord_out).r;
   float d = (t-0.5)*m;
   // Map the signed distance to mixing parameters for outline..foreground, transparent..opaque.
   float alpha = smoothstep(-0.5    , +0.5, d);
   float beta  = smoothstep(-M_SQRT2, -1.0, d);
   vec4 fg_c   = mix( outline_color, color, alpha );
   color_out   = vec4( fg_c.rgb, beta*fg_c.a );
   gl_FragDepth = -t+1.0;
}
