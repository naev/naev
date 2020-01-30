#version 130

in vec4 vertex;
uniform mat4 projection;
varying vec2 pos;

void main(void) {
   pos = vertex.xy - .5;
   gl_Position = projection * vertex;
}
