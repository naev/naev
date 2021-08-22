#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;

in vec2 pos;
out vec4 color_out;

void main(void) {
   float dplus = min(
      length( pos - vec2( clamp( pos.x, -0.8, +0.8 ), 0 ) ),
      length( pos - vec2( 0, clamp( pos.y, -0.8, +0.8 ) ) )
   );
   float dcirc = abs( length( pos ) - 0.8 );
   float sd    = min( dcirc, dplus );
   float alpha = 1. - smoothstep(0.05, 0.10, sd);
   float beta  = 1. - smoothstep(0.18, 0.20, sd);
   vec3 fg_c   = color.rgb * alpha;
   color_out   = vec4( fg_c, beta );
}

