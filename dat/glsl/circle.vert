in vec4 vertex;
uniform mat4 projection;
uniform float radius;
out vec2 pos;

void main(void) {
   pos = radius * vertex.xy;
   gl_Position = projection * vertex;
}
