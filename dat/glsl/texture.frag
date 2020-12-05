uniform vec4 color;
uniform sampler2D sampler;

in vec2 tex_coord;
out vec4 color_out;

void main(void) {
   color_out = color * texture(sampler, tex_coord);
}
