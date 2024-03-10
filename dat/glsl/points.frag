in vec4 colour;
out vec4 colour_out;

void main (void) {
   float d = length( gl_PointCoord*2.0-1.0 )-1.0;
   float a = smoothstep( 0.0, 0.5, -d );
   colour_out = colour;
   colour_out.a *= a;
}
