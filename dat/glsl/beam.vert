uniform mat4 tex_mat;
uniform mat4 projection;

in vec4 vertex;
out vec2 tex_coord;
out float pos;

void main(void) {
   pos = vertex.x;
   tex_coord = (tex_mat * vertex).st;
   gl_Position = projection * vertex;
}
