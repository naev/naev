#version 130

uniform mat4 projection;

uniform vec2 star_xy;
uniform vec2 wh;

uniform int shade_mode;
uniform vec2 xy;

in vec4 vertex;
in vec4 color;

out vec4 color_out;

void main(void) {
   float brightness = color[3];

   /* Calculate position */
   float b = 1./(9. - 10.*brightness);
   gl_Position = vertex;
   gl_Position.xy += star_xy * b;

   /* check boundaries */
   gl_Position.xy = mod(gl_Position.xy + wh/2, wh) - wh/2;

   /* Generate lines. */
   if (shade_mode != 0 && mod(gl_VertexID, 2) == 1) {
      gl_Position.xy += xy * brightness;
   }

   gl_Position = projection * gl_Position;

   color_out = color;
}
