uniform mat4 projection;

uniform vec2 star_xy;
uniform vec2 xy;
uniform vec3 dims;
uniform bool use_lines;

in vec4 vertex;
in float brightness;

out float brightness_out;

void main(void) {
   /* Calculate position */
   float b        = 1.0/(9.0 - 10.0*brightness);
   gl_Position    = vertex;
   gl_Position.xy+= star_xy * b;

   /* Check boundaries */
   gl_Position.xy = mod(gl_Position.xy + dims.xy/2.0, dims.xy) - dims.xy/2.0;

   /* Generate lines. */
   if (use_lines) {
      vec2 v = xy * brightness;
      gl_Position.xy += mod(float(gl_VertexID), 2.0) * v * dims.z;
   }

   gl_Position = projection * gl_Position;

   brightness_out = brightness;
}
