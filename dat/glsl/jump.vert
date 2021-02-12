uniform mat4 projection;
in vec4 vertex;
out vec2 pos;

void main(void) {
   pos = vertex.xy;
   gl_Position = projection * vertex;
}
