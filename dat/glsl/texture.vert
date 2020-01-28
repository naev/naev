#version 130

out vec2 tex_coord;

void main(void) {
   tex_coord = gl_MultiTexCoord0.st;
   gl_Position = ftransform();
}
