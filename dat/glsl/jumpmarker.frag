#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;

in vec2 pos;
out vec4 color_out;

void main(void) {
   vec2 uv = vec2( pos.y, pos.x );
   float m = 1.0 / dimensions.x;

   float db = sdBox( uv+vec2(0.0,0.10), vec2(0.10,0.6) );
   float dt = 2.0*sdTriangleIsosceles( 0.5*uv+vec2(0.0,0.40), vec2(0.45, 0.85) );
   float d = sdSmoothUnion( db, dt, 0.45 );
   d = abs(d);

   float alpha = smoothstep(-m, 0.0, -d);
   float beta  = smoothstep(-2.0*m, -m, -d);
   color_out   = color * vec4( vec3(alpha), beta );
}

