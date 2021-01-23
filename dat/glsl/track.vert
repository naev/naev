uniform mat4 projection;
in vec4 vertex;

void main(void) {
   pos = vertex.xy;
   gl_Position = projection * vertex;
}
