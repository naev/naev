#include "lib/sdf.glsl"

uniform vec4 colour;
uniform vec2 dimensions;
uniform int parami;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   const vec2 b = vec2( 1.0, 0.65 );
   const vec2 c = vec2(-0.35,0.0);
   float m = 1.0 / dimensions.x;

   float d = sdEgg( pos, b-2.0*m );
   vec2 cpos = pos+c;
   if (parami==1)
      d = max( -sdSegment( cpos, vec2(0.0), vec2(1.0,0.0) )+0.15, d );
   d = max( -sdCircle( cpos, 0.5 ), d );
   d = min( sdCircle( cpos, 0.2 ), d );

   float alpha = smoothstep(     -m, 0.0, -d );
   float beta  = smoothstep( -2.0*m,  -m, -d );
   colour_out   = colour * vec4( vec3(alpha), beta );
}
