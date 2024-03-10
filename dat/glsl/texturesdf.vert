uniform mat4 tex_mat;
uniform mat4 projection;
uniform vec2 dims;

in vec4 vertex;
out vec2 tex_coord;

void main(void) {
   tex_coord = (tex_mat * (vertex*0.5+0.5)).st;
   gl_Position = projection * vertex;
}
