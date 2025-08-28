#include "lib/sdf.glsl"

uniform vec4 colour;
uniform vec2 dimensions;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   vec2 uv = vec2( pos.y, pos.x );
   float m = 1.0 / dimensions.x;

   float db = sdBox( uv+vec2(0.0,0.6), vec2(0.2,0.6) );
   float dt = 2.0*sdTriangleIsosceles( 0.5*uv+vec2(0.0,0.2), vec2(0.5-m, 0.5) );
   float d = sdSmoothUnion( db, dt, 0.5 );
   d = max( d, abs(uv.y)-1.0+2.0*m );
   d = abs(d);

   float alpha = smoothstep(-m, 0.0, -d);
   float beta  = smoothstep(-2.0*m, -m, -d);
   colour_out   = colour * vec4( vec3(alpha), beta );
}
