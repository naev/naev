#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;

in vec2 pos;
out vec4 color_out;

void main(void) {
   vec2 uv = vec2( pos.y, pos.x );
   float m = 1.0 / dimensions.x;
   float d = sdCircle( uv, 1.0 );
   //d = min( d, sdSegment( uv, vec2(0.0), vec2(cos(t),sin(t)) ) );
   d = abs(d+2.0*m);
   const float a = radians(15.0);
   d = min( d, sdPie( uv, vec2(sin(a),cos(a)), 1.0-2.0*m ) );
   float alpha = smoothstep(    -m, 0.0, -d);
   float beta  = smoothstep(-2.0*m,  -m, -d);
   color_out   = color * vec4( vec3(alpha), beta );
}
