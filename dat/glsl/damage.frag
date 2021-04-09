#include "lib/simplex.glsl"

uniform sampler2D MainTex;
in vec4 VaryingTexCoord;
out vec4 color_out;

uniform float damage_strength = 0.0;
uniform float u_time = 0.0;

void main (void)
{
   vec2 uv = VaryingTexCoord.st;
   vec2 t = vec2( u_time, u_time );

   float noise1 = snoise( fract( uv + vec2(random(t), 0.25) * 10.0 ) * 0.75 );
   float noise2 = snoise( fract( uv + vec2(random(t), 0.78) * 10.0 ) * 0.5 );

   float offset = step( 0.5 * (noise1 + noise2), 0.5 );
   offset = (2.0 * offset + 1.0) * damage_strength;

   color_out.r = texture( MainTex, uv + vec2(offset,0.0) ).r;
   color_out.g = texture( MainTex, uv ).g;
   color_out.b = texture( MainTex, uv - vec2(offset,0.0) ).b;
   color_out.a = 1.0;
}
