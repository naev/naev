uniform vec4 colour;
uniform float inter;
uniform sampler2D sampler1;
uniform sampler2D sampler2;

in vec2 tex_coord;
out vec4 colour_out;

void main(void) {
   vec4 colour1 = texture(sampler1, tex_coord);
   vec4 colour2 = texture(sampler2, tex_coord);
   /* TODO probably do this zeroing when loading textures... */
   if (colour1.a <= 0.0)
      colour1.rgb = vec3(0.0);
   if (colour2.a <= 0.0)
      colour2.rgb = vec3(0.0);
   colour_out = colour * mix(colour2, colour1, inter);
}
