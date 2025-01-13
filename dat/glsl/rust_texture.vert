layout(std140) uniform TextureData {
   mat4 tex_mat;
   mat4 transform;
   vec4 colour;
};

layout(location = 0) in vec4 vertex;
out vec2 tex_coord;

void main(void) {
   tex_coord = (tex_mat * vertex).st;
   gl_Position = transform * vertex;
}
