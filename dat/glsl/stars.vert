uniform mat4 projection;

uniform vec2 star_xy;
uniform vec2 wh;
uniform vec2 xy;

in vec4 vertex;
in float brightness;

out float brightness_out;

void main(void) {
   /* Calculate position */
   float b = 1./(9. - 10.*brightness);
   gl_Position = vertex;
   gl_Position.xy += star_xy * b;

   /* check boundaries */
   gl_Position.xy = mod(gl_Position.xy + wh/2., wh) - wh/2.;

   /* Generate lines. */
   gl_Position.xy += mod(float(gl_VertexID), 2.) * xy * brightness;

   gl_Position = projection * gl_Position;

   brightness_out = brightness;
}
