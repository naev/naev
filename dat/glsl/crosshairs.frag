#include "lib/sdf.glsl"

uniform vec4 colour;
uniform vec2 dimensions;
uniform float paramf;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   float m = 1.0 / dimensions.x;
   float rad = 1.0 - paramf*m;
   float d = min(
      length( pos - vec2( clamp( pos.x, -rad, rad ), 0.0 ) ),
      length( pos - vec2( 0.0, clamp( pos.y, -rad, rad ) ) )
   );
   float alpha = smoothstep(-m, 0.0, -d);
   float beta  = smoothstep(-paramf*m, -m, -d);
   colour_out   = colour * vec4( vec3(alpha), beta );
}
