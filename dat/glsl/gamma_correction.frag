#include "lib/gamma.glsl"

uniform sampler2D MainTex;
uniform float gamma = 1.0;
in vec4 VaryingTexCoord;
out vec4 colour_out;

void main (void)
{
   colour_out = texture( MainTex, VaryingTexCoord.st );
   colour_out.rgb = pow( colour_out.rgb, vec3(1.0 / gamma) );
}
