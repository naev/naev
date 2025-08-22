in vec4 vertex;
in vec4 vertex_colour;
uniform mat4 projection;
out vec4 colour;

void main(void) {
   colour = vertex_colour;
   gl_Position = projection * vertex;
}
