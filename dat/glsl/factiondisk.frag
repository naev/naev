uniform vec4 colour;
uniform float paramf;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   colour_out = colour;

   float dist = length(pos);
   colour_out.a *= exp( 1.0 / (dist+1.0) - 0.5) - 1.0;
   colour_out.a *= smoothstep( 0.5*paramf, paramf, dist );
}
