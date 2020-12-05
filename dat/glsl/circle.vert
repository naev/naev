in vec4 vertex;
uniform mat4 projection;
uniform float radius;
out vec2 pos;

void main(void) {
   pos = 2. * radius * (vertex.xy - .5);
   gl_Position = projection * vertex;
}
