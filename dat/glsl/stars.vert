#version 130

uniform float star_x;
uniform float star_y;
uniform float w;
uniform float h;

uniform int shade_mode;
uniform float x;
uniform float y;

out vec4 color;

void main(void) {
   float brightness = gl_Color[3];
   gl_Position = gl_Vertex;

   /* Calculate position */
   float b = 1./(9. - 10.*brightness);
   gl_Position[0] += star_x*b;
   gl_Position[1] += star_y*b;

   /* check boundaries */
   // TODO: Why isn't this working?
   //gl_Position[0] = mod(gl_Position[0] - w/2, w) + w/2;
   //gl_Position[1] = mod(gl_Position[1] - h/2, h) + h/2;

   /* Generate lines. */
   if (shade_mode != 0 && mod(gl_VertexID, 2) == 1) {
      gl_Position[0] += x*brightness;
      gl_Position[1] += y*brightness;
   }

   gl_Position = gl_ModelViewProjectionMatrix * gl_Position;

   color = gl_Color;
}
