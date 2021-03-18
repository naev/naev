uniform sampler2D MainTex;
in vec4 VaryingTexCoord;
out vec4 color_out;

void main (void)
{
   color_out = texture( MainTex, VaryingTexCoord.st );
}
