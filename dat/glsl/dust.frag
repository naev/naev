uniform bool use_lines;
uniform float dim;

in float brightness_frag;
in vec2 pos_frag;
out vec4 colour_out;

void main (void) {
   colour_out = vec4( 1.0, 1.0, 1.0, brightness_frag );
   colour_out.a *= smoothstep( 0.0, 1.0, 1.0-length(pos_frag) );

   if (use_lines)
      colour_out.a *= dim;
}
