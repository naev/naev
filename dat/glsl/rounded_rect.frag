#include "lib/sdf.glsl"
uniform vec4 colour;
uniform vec4 dimensions; // size, corners radius
uniform float paramf; // line_width

in vec2 pos;
out vec4 colour_out;

void main(void) {
    vec2 uv = pos * 2.0 - 1.0;

    float d = paramf + sdBoxRound(
                uv * dimensions.xy / 2.0,
                dimensions.xy / 2.0,
                dimensions.zw);
    colour_out = colour;
    if (paramf > 0.) {
        d = abs(d);
        colour_out.a = smoothstep(
                -paramf,
                -paramf / 2.,
                -d);
    } else
        colour_out.a = smoothstep(0, 0.5, -d);
}
