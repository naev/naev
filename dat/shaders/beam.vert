uniform mat4 projection;
uniform vec2 dimensions;
in vec4 vertex;
out vec2 pos_tex;
out vec2 pos_px;

void main(void) {
   pos_tex = vec2( vertex.x, 2.0 * vertex.y - 1.0 );
   pos_px = vertex.xy * dimensions;
   gl_Position = projection * vertex;
}
