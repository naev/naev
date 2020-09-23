#version 140

in vec4 vertex;
in vec4 vertex_color;
uniform mat4 projection;
out vec4 color;

void main(void) {
   color = vertex_color;
   gl_Position = projection * vertex;
}
