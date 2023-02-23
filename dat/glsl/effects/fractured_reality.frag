uniform vec3 dimensions;
uniform sampler2D u_tex;

uniform float u_timer   = 0.0;
uniform float u_elapsed = 0.0;

in vec2 tex_coord;
out vec4 colour_out;

void main(void) {
   colour_out = texture( u_tex, tex_coord );
   colour_out.rgb = 1.0 - colour_out.rgb;
}
