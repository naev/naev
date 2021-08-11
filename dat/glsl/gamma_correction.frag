uniform sampler2D MainTex;
uniform float gamma = 1.0;
in vec4 VaryingTexCoord;
out vec4 color_out;

void main (void)
{
   color_out = texture( MainTex, VaryingTexCoord.st );

   color_out.rgb = mix(
      1.055*pow(color_out.rgb,vec3(1.0/2.4))-0.055,
      color_out.rgb*12.92,
      step(color_out.rgb, vec3(0.0031308)));

   color_out.rgb = pow( color_out.rgb, vec3(1.0 / gamma) );
}
