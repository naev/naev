#version 130

uniform mat4 tex_mat;
uniform mat4 projection;

out vec2 tex_coord;

void main(void) {
   tex_coord = (tex_mat * gl_Vertex).st;
   gl_Position = projection * gl_Vertex;
}
