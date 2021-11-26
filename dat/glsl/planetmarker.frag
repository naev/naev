#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;
uniform int parami;

in vec2 pos;
out vec4 color_out;

void main(void) {
   float m = 1.0 / dimensions.x;
   float rad = 1.0 - 2.0*m;
   float d;
   if (parami==0)
      d           = abs( sdCircle( pos, rad ) );
   else {
      float dplus = min(
         length( pos - vec2( clamp( pos.x, -rad, rad ), 0.0 ) ),
         length( pos - vec2( 0.0, clamp( pos.y, -rad, rad ) ) )
      );
      float dcirc = abs( sdCircle( pos, rad ) );
      d           = min( dcirc, dplus );
   }
   float alpha = smoothstep(    -m, 0.0, -d);
   float beta  = smoothstep(-2.0*m,  -m, -d);
   color_out   = color * vec4( vec3(alpha), beta );
}
