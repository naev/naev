#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;

in vec2 pos;
out vec4 color_out;

void main(void) {
   float m = 1.0 / dimensions.x;
   float r = 1.0 - 2.0*m;
   float dplus = min(
      length( pos - vec2( clamp( pos.x, -r, r ), 0.0 ) ),
      length( pos - vec2( 0.0, clamp( pos.y, -r, r ) ) )
   );
   float dcirc = abs(sdCircle( pos, r ));
   float d     = min( dcirc, dplus );
   float alpha = smoothstep(-m, 0.0, -d);
   float beta  = smoothstep(-2.0*m, -m, -d);
   color_out   = color * vec4( vec3(alpha), beta );
}

