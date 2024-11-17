uniform vec4 colour;
uniform float inter;
uniform sampler2D sampler1;
uniform sampler2D sampler2;

in vec2 tex_coord;
out vec4 colour_out;

void main(void) {
   vec4 colour1 = colour * texture(sampler1, tex_coord);
   vec4 colour2 = colour * texture(sampler2, tex_coord);
   colour_out = mix(colour2, colour1, inter);
}
