#include "lib/sdf.glsl"

uniform vec4 colour;
uniform vec2 dimensions;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   vec2 uv = pos * dimensions;
   float d = sdBox( uv, dimensions-vec2(2.0) );
   float alpha = smoothstep(-1.0,  0.0, -d);
   float beta  = smoothstep(-2.0, -1.0, -d);
   colour_out   = colour * vec4( vec3(alpha), beta );
}
