#include "lib/sdf.glsl"

uniform vec4 colour;
uniform vec2 dimensions;
uniform float dt;

in vec2 pos;
out vec4 colour_out;

void main(void)
{
   float a     = 1.0 - 0.5*fract( dt/2.0 );
   float d     = sdCircle( pos*dimensions, a*dimensions.x-1.0 );
   float alpha = smoothstep(-1.0, 0.0, -d);
   colour_out   = colour;
   colour_out.a *= alpha;
   colour_out.a *= 0.6 + 0.4*length(pos);
}
