uniform sampler2D sampler;

in vec2 tex_coord;

void main(void) {
   gl_FragDepth = texture( sampler, tex_coord ).x;
}
