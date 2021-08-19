uniform mat4 projection_view;
uniform mat4 projection_model;

in vec4 vertex;
in vec3 vertex_normal;
in vec2 vertex_tex;
out vec2 tex_coord;
out vec3 normal;

void main(void) {
   tex_coord   = vec2(vertex_tex.x, -vertex_tex.y);
   normal      = mat3(projection_model) * vertex_normal;
   gl_Position = projection_view * projection_model * vertex;
}
