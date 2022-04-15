const mat4 S = mat4(
    2.0,  0.0, 0.0, 0.0,
    0.0,  2.0, 0.0, 0.0,
    0.0,  0.0, 1.0, 0.0,
   -0.5, -0.5, 0.0, 1.0
);

uniform mat4 tex_mat;
uniform mat4 projection;

in vec4 vertex;
out vec2 tex_coord;

void main(void) {
   tex_coord = (tex_mat * S * vertex).st;
   gl_Position = projection * S * vertex;
}
