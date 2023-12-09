#include "lib/sdf.glsl"

uniform vec4 colour;
uniform vec2 dimensions;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   vec2 uv = vec2( pos.y, pos.x );
   float m = 1.0 / dimensions.x;
   float d = sdTriangleEquilateral( uv*1.15  ) / 1.15;
   d = abs(d+2.0*m);
   float alpha = smoothstep(    -m, 0.0, -d);
   float beta  = smoothstep(-2.0*m,  -m, -d);
   colour_out   = colour * vec4( vec3(alpha), beta );
}
