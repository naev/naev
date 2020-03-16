#version 130

uniform mat4 projection;
uniform vec2 wh;

in vec4 vertex;
out vec2 pos;
out float ratio;

void main(void) {
   pos = wh * (vertex.xy - .5);
   ratio = vertex.y;
   gl_Position = projection * vertex;
}
