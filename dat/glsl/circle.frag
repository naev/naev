#include "lib/sdf.glsl"

uniform vec4 colour;
uniform vec2 dimensions;
uniform int parami;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   float d = sdCircle( pos*dimensions, dimensions.x-1.0 );
   if (parami==0)
      d = abs(d);
   /*
   float alpha = smoothstep(-m, 0.0, -d);
   float beta  = smoothstep(-2.0*m, -m, -d);
   colour_out   = colour * vec4( vec3(alpha), beta );
   */
   float alpha = smoothstep(-1.0, 0.0, -d);
   colour_out   = colour;
   colour_out.a *= alpha;
}
