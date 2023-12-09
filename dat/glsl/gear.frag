#include "lib/sdf.glsl"
#include "lib/math.glsl"

uniform vec4 colour;
uniform vec2 dimensions;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   vec2 uv = vec2( pos.y, pos.x );
   float m = 1.0 / dimensions.x;

   float d = sdCircle( uv, 0.75 );

   vec2 auv = abs(uv);
   if (auv.y < auv.x)
      auv.xy = vec2( auv.y, auv.x );
   const float s = sin( -M_PI/8.0 );
   const float c = cos( -M_PI/8.0 );
   const mat2 R = mat2( c, s, -s, c );
   d = sdSmoothUnion( d, sdBox( auv*R, vec2(0.1,1.0-m) )-0.025, 0.2 );

   d = max( d, -sdCircle( uv, 0.6 ) );
   d = min( d, sdCircle( uv, 0.25 ) );

   float alpha = smoothstep(-m, 0.0, -d);
   float beta  = smoothstep(-2.0*m, -m, -d);
   colour_out   = colour * vec4( vec3(alpha), beta );
}
