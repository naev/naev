#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;
uniform float paramf;

in vec2 pos;
out vec4 color_out;

void main(void) {
   vec2 uv     = pos * dimensions;
   float d     = sdBox( uv, dimensions-vec2(paramf) );
   float alpha = smoothstep(   -1.0,  0.0, -d);
   float beta  = smoothstep(-paramf, -1.0, -d);
   color_out   = color * vec4( vec3(alpha), beta );
   //color_out = vec4(vec3(d),1.0);
}
