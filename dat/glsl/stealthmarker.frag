#include "lib/math.glsl"
#include "lib/sdf.glsl"

uniform vec4 colour;
uniform vec2 dimensions;
uniform float paramf;

in vec2 pos;
out vec4 colour_out;

void main (void) {
   float a = paramf * M_PI;
   float c = cos(a);
   float s = sin(a);
   vec2 uv = vec2(pos.x, -pos.y);
   uv = mat2(c,-s,s,c) * uv;
   float d = sdPie( uv*dimensions, vec2(s,c), dimensions.x-1.0 );

   float alpha = smoothstep(-1.0, 0.0, -d);
   colour_out   = colour;
   colour_out.a *= alpha;
}
