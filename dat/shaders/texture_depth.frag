uniform vec4 colour;
uniform sampler2D sampler;
uniform sampler2D depth;

in vec2 tex_coord;
out vec4 colour_out;

void main(void) {
   colour_out = colour * texture( sampler, tex_coord );
   gl_FragDepth = texture( depth, tex_coord ).x;
}
