#version 130

in int shade_mode;
out vec4 color;

void main(void) {
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	color = gl_Color;
}
