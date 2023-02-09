uniform bool use_lines;
uniform float dim;

in float brightness_frag;
out vec4 colour_out;

void main (void) {
   colour_out = vec4( 1.0, 1.0, 1.0, brightness_frag );
   /*
   if (!use_lines) {
      float d = length( gl_PointCoord*2.0-1.0 )-1.0;
      float a = smoothstep( 0.0, 1.0, -d );
      colour_out.a *= a;
   }
   */
   colour_out.a *= dim;
}
