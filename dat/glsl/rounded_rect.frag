#include "lib/sdf.glsl"
uniform vec4 colour;
uniform vec4 dimensions;
uniform int parami;

in vec2 pos;
out vec4 colour_out;
float sdBoxRound(vec2 p, vec2 b, vec2 r)
{
    float diff = b.x - r.x;
    float coef = min(1.0, max(0.0, (abs(p.x) - diff) / (b.x - diff)));
    float rf = coef * r.y + (1.0 - coef) * r.x;
    vec2 d = abs(p) - b + rf;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - rf;
}
void main(void) {
    vec2 uv = pos * 2.0 - 1.0;
    float d = sdBoxRound(uv * dimensions.xy, dimensions.xy, dimensions.zw * 2.0);
    colour_out = colour;
    if (parami > 0.0)
        d = abs(d + parami);
    float thick = parami > 0 ? parami : 1.0;
    colour_out.a = smoothstep(-thick / 2.0, thick / 2.0, -d);
}
