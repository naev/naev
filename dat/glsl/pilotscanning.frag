#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;

in vec2 pos;
out vec4 colour_out;

void main (void)
{
   vec2 uv = vec2( pos.y, pos.x );
   float m = 1.0 / dimensions.x;
   float d = sdCircle( uv, 1.0 );
   d = abs(d+2.0*m);
   const float a = radians(15.0);
   const vec2 va = vec2(sin(a),cos(a));
   float dpie = sdPie( uv, va, 1.0-2.0*m );
   d = min( d, dpie );
   float alpha = smoothstep(    -m, 0.0, -d);
   float beta  = smoothstep(-2.0*m,  -m, -d);

   if (dpie < m)
      colour_out = vec4( vec3(0.1,0.3,0.7), color.a );
   else
      colour_out = color;
   colour_out *= vec4( vec3(alpha), beta );
}
