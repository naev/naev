uniform mat4 projection;

in vec4 vertex;
in vec2 tex_coord;
out vec2 tex_coord_out;

void main(void) {
   tex_coord_out = tex_coord;
   gl_Position = projection * vertex;
}
