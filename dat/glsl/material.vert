uniform mat4 projection;

in vec4 vertex;
in vec3 vertex_normal;
in vec2 vertex_tex;
out vec2 tex_coord;
out vec3 normal;

void main(void) {
   tex_coord   = vec2(vertex_tex.x, -vertex_tex.y);
   normal      = vertex_normal;
   gl_Position = projection * vertex;
}
