#include "lib/sdf.glsl"
#include "lib/math.glsl"

uniform vec4 colour;

in vec2 pos;
out vec4 colour_out;

void main(void)
{
   float d = 1.0-distance( pos, vec2(0.0,0.0) );
   colour_out = colour;
   colour_out.a *= pow(d,0.75);
}
