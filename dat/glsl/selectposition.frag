#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;

in vec2 pos;
out vec4 color_out;

void main(void) {
   float m = 1.0 / dimensions.x;

   vec2 uv = abs( pos );
   if (uv.y < uv.x)
      uv.xy = uv.yx;
   float m2 = m*2.0;
   float d = sdSegment( uv, vec2(0.2+m,1.0-m2), vec2(1.0,1.0)-m2 );

   float alpha = smoothstep(     -m, 0.0, -d );
   float beta  = smoothstep( -2.0*m,  -m, -d );
   color_out   = color * vec4( vec3(alpha), beta );
}


