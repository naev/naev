#version 130

out vec2 tex_coord;
out vec4 color;

void main(void) {
   tex_coord = gl_MultiTexCoord0.st;
   gl_Position = ftransform();
   color = gl_Color;
}
