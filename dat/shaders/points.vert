in vec4 vertex;
in vec4 vertex_colour;
uniform mat4 projection;
uniform float pointsize;
out vec4 colour;

void main(void) {
   colour = vertex_colour;
   gl_Position = projection * vertex;
   gl_PointSize = pointsize;
}
