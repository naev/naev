uniform sampler2D sampler;

in vec2 tex_coord;
out vec4 colour_out;

void main (void)
{
   float r = texture( sampler, tex_coord ).r;
   gl_FragDepth = r;
}
