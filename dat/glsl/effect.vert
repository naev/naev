uniform mat4 tex_mat;
uniform mat4 projection;

in vec4 vertex;
out vec2 tex_coord;
out vec2 tex_scale;

void main(void) {
   tex_coord = (tex_mat * vertex).st;
   tex_scale = vec2( tex_mat[0][0], tex_mat[1][1] );
   gl_Position = projection * vertex;
}
