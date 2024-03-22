#include "lib/sdf.glsl"
uniform vec4 colour;
uniform vec2 dimensions;
uniform int parami;

in vec2 pos;
out vec4 colour_out;

void main(void) {
    vec2 uv = pos * 2.0 - 1.0;
    float d = sdBox(uv * dimensions.xy, dimensions.xy);
    colour_out = colour;
    if (parami > 0.0)
        d = abs(d + parami);
    float thick = parami > 0 ? parami : 1.0;
    colour_out.a = smoothstep(0.5 - thick, 0.5, -d);
}
