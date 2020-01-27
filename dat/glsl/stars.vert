#version 130

uniform vec2  star_xy;
uniform float w;
uniform float h;

uniform int shade_mode;
uniform vec2 xy;

out vec4 color;

void main(void) {
   float brightness = gl_Color[3];

   /* Calculate position */
   float b = 1./(9. - 10.*brightness);
   gl_Position = gl_Vertex + vec4(star_xy, 0, 0) * b;

   /* check boundaries */
   gl_Position[0] = mod(gl_Position[0] + w/2, w) - w/2;
   gl_Position[1] = mod(gl_Position[1] + h/2, h) - h/2;

   /* Generate lines. */
   if (shade_mode != 0 && mod(gl_VertexID, 2) == 1) {
      gl_Position += vec4(xy, 0, 0) * brightness;
   }

   gl_Position = gl_ModelViewProjectionMatrix * gl_Position;

   color = gl_Color;
}
