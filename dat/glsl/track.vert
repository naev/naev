uniform mat4 projection;
in vec4 vertex;
out vec2 pos;

void main(void) {
   pos = vec2(vertex.x, 2.*vertex.y-1.);
   //pos = vec2(2.*vertex.x-.5, 2.*vertex.y-1.);
   //pos = vec2(vertex.x*(1.+2.*rad)-rad, 2.*vertex.y-1.);
   //pos = vec2( vertex.x*(1.+2.*rad)-rad, 2.*rad*vertex.y-rad );
   //pos = vec2((vertex.x-rad)*(1.+2.*rad), 2.*vertex.y-1.);
   gl_Position = projection * vertex;
}
