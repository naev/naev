uniform vec4 Ka;
uniform vec4 Kd;
uniform vec4 Ks;
uniform float Ns;
uniform sampler2D sampler;

in vec2 tex_coord_out;
out vec4 color_out;

void main(void) {
   color_out = Kd * texture(sampler, tex_coord_out); /* TODO placeholder!!! */
}
