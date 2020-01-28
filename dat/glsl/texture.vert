#version 130

uniform mat4 projection_matrix;

out vec2 tex_coord;

void main(void) {
   tex_coord = gl_MultiTexCoord0.st;
   gl_Position = projection_matrix * gl_Vertex;
}
