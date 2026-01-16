uniform mat4 projection;
uniform vec2 dimensions;
in vec4 vertex;
out vec2 pos;
out vec2 pos_tex;
out vec2 pos_px;

void main(void) {
   pos = vertex.xy;
   pos_tex = vec2( pos.x, 2.0 * pos.y - 1.0 );
   pos_px = pos * dimensions;
   gl_Position = projection * vertex;
}
