#include "lib/math.glsl"
#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;
uniform float dt;

in vec2 pos;
out vec4 color_out;

void main(void)
{
   float d     = sdCircle( pos*dimensions, dimensions.x );
   float alpha = smoothstep(-1.0, 0.0, -d);
   color_out   = color;
   color_out.a *= alpha;
   color_out.a *= 0.3 + 0.1 * sin( dt * M_PI ) + 0.6*length(pos);
}
