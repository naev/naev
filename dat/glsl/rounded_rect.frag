#include "lib/sdf.glsl"
uniform vec4 colour;
uniform vec4 dimensions;
uniform float paramf;

in vec2 pos;
out vec4 colour_out;

void main(void) {
    float thick = floor(paramf);
    float smoothy = mod(paramf, 1.0);

    vec2 uv = pos * 2.0 - 1.0;

    float d = thick + sdBoxRound(
                uv * dimensions.xy / 2.0,
                dimensions.xy / 2.0,
                dimensions.zw);
    colour_out = colour;
    if (thick > 0.)
        d = abs(d) - thick / 2.0;
    if (smoothy != 0.0)
        colour_out.a = smoothstep(
                -(1.0 - smoothy) * thick,
                smoothy * thick,
                -d);
    else
        colour_out.a = smoothstep(-0.5, 0.5, -d);
    if (colour_out.a > 0.0 && colour_out.a < 0.5)
       colour_out.a = 0.5;
}
