#version 140

in vec4 vertex;
uniform mat4 projection;

void main(void) {
   gl_Position = projection * vertex;
}
