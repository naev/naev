in vec4 vertex;
out vec2 tex_coord;

void main (void)
{
   tex_coord = vertex.st;
   gl_Position = vertex*0.5 + 0.5;
}
