layout(std140) uniform TextureData {
   mat4 tex_mat;
   mat4 transform;
   vec4 colour;
};

uniform sampler2D sampler;

in vec2 tex_coord;
layout(location = 0) out vec4 colour_out;

void main(void) {
   colour_out = colour * texture(sampler, tex_coord);
}
