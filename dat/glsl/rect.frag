#include "lib/sdf.glsl"
uniform vec4 colour;
uniform vec2 dimensions; // size
uniform float paramf; // line_width

in vec2 pos;
out vec4 colour_out;

void main(void) {
    vec2 uv = pos * 2.0 - 1.0;
    float d = paramf + sdBox(
                uv * dimensions.xy / 2.,
                dimensions.xy / 2.);
    colour_out = colour;
    if (paramf > 0.0) {
        d = abs(d);
        colour_out.a = smoothstep(
                -paramf,
                -paramf / 2.0,
                -d);
    } else
        colour_out.a = smoothstep(0., 0.5, -d);
}
