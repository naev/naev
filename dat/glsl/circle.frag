#include "lib/sdf.glsl"

uniform vec4 colour;
uniform vec2 dimensions;
uniform float paramf;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   float d = sdCircle( pos*dimensions, dimensions.x-1.0 );
   if (paramf > 0)
      d = abs(d);
   float line_width = paramf > 0.0 ? paramf : 1.0;

   /*
   float alpha = smoothstep(-m, 0.0, -d);
   float beta  = smoothstep(-2.0*m, -m, -d);
   colour_out   = colour * vec4( vec3(alpha), beta );
   */
   float alpha = smoothstep(-line_width, 0.0, -d);
   colour_out   = colour;
   colour_out.a *= alpha;
}
