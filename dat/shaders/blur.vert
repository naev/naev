in vec4 vertex;
out vec2 tex_coord;

void main (void)
{
   tex_coord = vertex.st;
   gl_Position = vertex*2.0-1.0;
}
