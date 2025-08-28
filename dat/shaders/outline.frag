uniform vec4 colour;
uniform vec2 border;
in vec2 pos;
out vec4 colour_out;

void main(void) {
   vec2 uv = pos * 2.0 - 1.0;
   if (abs(uv.x) < border.x && abs(uv.y) < border.y)
      discard;
   colour_out = colour;
}
