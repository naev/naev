#version 140

uniform mat4 projection;
in vec4 vertex;

void main(void) {
   gl_Position = projection * vertex;
}
