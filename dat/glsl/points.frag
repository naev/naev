in vec4 color;
out vec4 color_out;

void main (void) {
   float d = length( gl_PointCoord*2.0-1.0 )-1.0;
   float a = smoothstep( 0.0, 0.5, -d );
   color_out = color;
   color_out.a *= a;
}
