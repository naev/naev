#include "lib/sdf.glsl"
uniform vec4 colour;
uniform vec4 dimensions; // used for borders size.
uniform int parami; //filled?
//uniform vec2 size;  // I need a size variable?

in vec2 pos;
out vec4 colour_out;
float sdBoxRound ( vec2 p, vec2 b, vec2 r )
{
   float diff = b.x - r.x;
   float coef = min(1.0, max(0.0, (abs(p.x) - diff) / (b.x - diff)));
   float rf = coef * r.y + (1.0 - coef) * r.x;
   vec2 d = abs(p) - b + rf;
   return length(max(d, 0.0))+ min(max(d.x, d.y), 0.0)- rf;
}
void main(void) {
   vec2 uv = pos * 2.0 - 1.0;
   float d = sdBoxRound(uv * dimensions.xy, dimensions.xy, dimensions.zw * 2.0);
   colour_out = colour;
   if (parami == 0) {
      d = abs(d+5);
      colour_out.a = smoothstep(-5, 0.0, -d);
   } else {
      colour_out.a = smoothstep(-0.5, 0.5, -d);
   }
}
