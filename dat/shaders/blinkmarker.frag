#include "lib/math.glsl"
#include "lib/sdf.glsl"

uniform vec4 colour;
uniform vec2 dimensions;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   float m = 1.0 / dimensions.x;

   const float w = 0.20;
   const float h = 0.05;

   vec2 uv = abs(pos);
   const float s = sin(M_PI/4.0);
   const float c = cos(M_PI/4.0);
   const mat2 R = mat2( c, s, -s, c );
   uv = uv - (vec2(1.0-w*M_SQRT1_2)-2.0*m);
   uv = R * uv;

   float d = sdRhombus( uv, vec2(h,w) );

   float alpha = smoothstep(-m, 0.0, -d);
   float beta  = smoothstep(-2.0*m, -m, -d);
   colour_out   = colour * vec4( vec3(alpha), beta );
}
