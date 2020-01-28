#version 130

uniform mat4 projection;

out vec2 tex_coord;

void main(void) {
   tex_coord = gl_MultiTexCoord0.st;
   gl_Position = projection * gl_Vertex;
}
