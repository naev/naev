#version 130

uniform vec4 color;
out vec4 color_out;
in vec2 pos;

void main(void) {
   float dist = length(pos);
   if (dist < .5) {
      color_out = color;
   } else {
      discard;
   }
}
