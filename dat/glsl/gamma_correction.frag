#include "lib/gamma.glsl"

uniform sampler2D MainTex;
uniform float gamma;
in vec4 VaryingTexCoord;
out vec4 color_out;

void main (void)
{
   color_out = texture( MainTex, VaryingTexCoord.st );
   color_out.rgb = pow( color_out.rgb, vec3(1.0 / gamma) );
}
