uniform sampler2D sampler;

in float pos;
in vec2 tex_coord;
out vec4 color_out;

void main(void) {
   color_out = texture(sampler, tex_coord);
   if (pos < .1) 
      color_out.a *= pos / .1;
   else if (pos > .8)
      color_out.a *= 1. - ((pos - .8) / .2);
}
