uniform vec4 color;

in vec2 pos;
out vec4 color_out;

void main(void) {
   vec2 rpos = pos * 2.0 - 1.0;
   color_out = color;
   
   float dist = length(rpos);
   color_out.a *= exp( 1.0 / (dist+1.0) - 0.5) - 1.0;
}

