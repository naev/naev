#version 140

uniform vec4 color;
uniform float inter;
uniform sampler2D sampler1;
uniform sampler2D sampler2;

in vec2 tex_coord;
out vec4 color_out;

void main(void) {
   vec4 color1 = color * texture(sampler1, tex_coord);
   vec4 color2 = color * texture(sampler2, tex_coord);
   color_out = mix(color2, color1, inter);
}
