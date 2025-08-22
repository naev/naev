uniform mat4 projection;
in vec4 vertex;
out vec2 localpos;

void main(void) {
   localpos = 2.0*vertex.xy - vec2(1.0);
   gl_Position = projection * vertex;
}
