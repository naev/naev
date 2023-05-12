#include "lib/sdf.glsl"
#include "lib/math.glsl"

uniform vec4 color;
uniform vec2 dimensions;
uniform float paramf;
uniform vec4 paramv;

in vec2 pos;
out vec4 color_out;

void main(void)
{
   vec2 uv     = pos * dimensions;

   float d     = sdBox( uv, dimensions );

   float inner = smoothstep( 2.0, 3.5, -d );

   float alpha = smoothstep( 0.0, 1.0, -d );
   float beta  = smoothstep( 1.0, 2.0, -d );

   vec4 base = (pos.y*0.5+0.5 < paramf) ? paramv : vec4(0.0, 0.0, 0.0, 1.0);

   color_out   = mix( color, base, inner ) * vec4( vec3(alpha), beta );
}
