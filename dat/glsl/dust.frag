uniform bool use_lines;
uniform float dim;
uniform vec3 dims;

in float brightness_frag;
in vec2 pos_frag;
in float length_frag;
out vec4 colour_out;

void main (void) {
   vec2 pos = pos_frag;
   colour_out = vec4( 1.0, 1.0, 1.0, brightness_frag );

   if (use_lines) {
      if (length_frag < dims.x)
         pos.x = length_frag / dims.x - 1.0;
      else
         pos.x = (length_frag-dims.x) / (dims.x+dims.z);
      colour_out.a *= dim;
   }

   colour_out.a *= smoothstep( 0.0, 1.0, 1.0-length(pos) );
}
