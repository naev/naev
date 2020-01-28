#version 130

uniform sampler2D sampler;

in vec2 tex_coord;
in vec4 color;
out vec4 color_out;

void main(void) {
   color_out = color * texture2D(sampler, tex_coord);
}
