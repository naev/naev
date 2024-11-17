uniform vec4 colour;
uniform sampler2D sampler;

in vec2 tex_coord;
out vec4 colour_out;

void main(void) {
   colour_out = colour * texture(sampler, tex_coord);
}
