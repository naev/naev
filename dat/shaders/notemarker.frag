#include "lib/sdf.glsl"
#include "lib/math.glsl"

uniform vec4 colour;
uniform vec2 dimensions;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   const vec2 b = vec2( 0.8, 0.25 );
   const vec2 c = vec2( -0.15, 0.0);
   const mat2 R = mat2(
      M_SQRT1_2, M_SQRT1_2,
     -M_SQRT1_2, M_SQRT1_2
   );
   float m = 1.0 / dimensions.x;

   float d = sdEgg( pos+vec2(0.2,0.0), b-2.0*m );
   vec2 cpos = R*(pos+c)+vec2(m,0.0);
   d = min( d, sdBox( cpos, vec2(0.4) )-0.16 );
   d = max( d, -sdSegment( abs(cpos), vec2(-0.38,0.33), vec2(0.38,0.33) )+0.08 );
   d = max( d, -sdSegment( cpos, vec2(-0.38,0.0), vec2(0.38,0.0) )+0.08 );

   float alpha = smoothstep(     -m, 0.0, -d );
   float beta  = smoothstep( -2.0*m,  -m, -d );
   colour_out   = colour * vec4( vec3(alpha), beta );
}
