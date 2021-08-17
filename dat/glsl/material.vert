uniform mat4 trans;

in vec4 vertex;
in vec3 normal;
in vec2 tex;
out vec2 tex_out;
out vec3 normal_out;

void main(void) {
   tex_out = vec2(tex.x, -tex.y);
   normal_out = normal;
   gl_Position = trans * vertex;
}
