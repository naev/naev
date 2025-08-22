uniform mat4 projection;
uniform vec3 dims;
uniform vec3 screen;
uniform vec2 offset_xy;
uniform bool use_lines;

in vec4 shape;
in vec4 vertex;
in float brightness;
out float brightness_frag;
out float length_frag;
out vec2 pos_frag;

void main(void) {
   vec4 center = vertex;
   float b = 1.0/(9.0 - 10.0*brightness);
   center.xy += offset_xy * b;
   center.xy = mod(center.xy + screen.xy/2.0, screen.xy) - screen.xy/2.0;
   float r = dims.x * 3.0;
   vec2  p = shape.xy * r;

   /* Calculate position */
   if (use_lines) {
      float a = dims.y;
      float l = dims.z;
      float c = cos(a);
      float s = sin(a);
      mat2 M  = mat2( c, s, -s, c );

      if (p.x < 0.0) {
         length_frag = -r-l;
         p.x -= l;
      } else {
         length_frag = r;
      }
      gl_Position = projection * (center + vec4( M*vec2(p.x, p.y), 0.0, 0.0));
      // Lower brightness sort of heuristically based on length which starts at roughly 0
      brightness_frag = brightness / (1.0 + l*0.1);

   } else {
      gl_Position = projection * (center + vec4( p.x, p.y, 0.0, 0.0));
      length_frag = 0.0;
      brightness_frag = brightness;
   }

   pos_frag = shape.xy;
}
