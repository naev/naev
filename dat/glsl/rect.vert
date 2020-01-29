#version 130

uniform mat4 projection;

void main(void) {
   gl_Position = projection * gl_Vertex;
}
