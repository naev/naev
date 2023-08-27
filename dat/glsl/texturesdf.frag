#include "lib/math.glsl"

uniform vec4 color;
uniform sampler2D sampler;
uniform float m;
uniform float outline;

in vec2 tex_coord;
out vec4 color_out;

const vec4 OUTLINE_COLOUR = vec4( vec3(0.0), 1.0 );

void main(void) {
   // d is the signed distance to the glyph; m is the distance value corresponding to 1 "pixel".
   float d  = 2.0*texture(sampler, tex_coord).r - 1.0;
   // Map the signed distance to mixing parameters for outline..foreground, transparent..opaque.
   float alpha = smoothstep( 1.0*m, 2.0*m, d);
   float beta  = smoothstep( 0.0*m, 1.0*m, d);
   vec4 fg_c   = mix( OUTLINE_COLOUR, color, alpha );
   color_out   = vec4( fg_c.rgb, beta*fg_c.a );
   //gl_FragDepth = d*0.5 + 0.5;

   color_out   = vec4( vec3(1.0), d );
}
