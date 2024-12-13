uniform bool use_lines;
uniform vec3 dims;

in float brightness_frag;
in vec2 pos_frag;
in float length_frag;
out vec4 colour_out;

void main (void) {
   vec2 pos = pos_frag;
   colour_out = vec4( 1.0, 1.0, 1.0, brightness_frag );

   if (use_lines) {
      float l = dims.z;
      float r = 3.0* dims.x;
      if (length_frag > 0.0) {
         pos.x = length_frag / r;
      } else {
         pos.x = length_frag / (r+l);
      }
   }

   colour_out.a *= smoothstep( 0.0, 1.0, 1.0-length(pos) );
}
