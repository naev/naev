#version 130

uniform vec4 color1;
uniform vec4 color2;
uniform vec2 wh;
uniform float corner_radius;

in float ratio;
in vec2 pos;
out vec4 color_out;

void main(void) {
   color_out = mix(color1, color2, ratio);

   float d1 = corner_radius - distance(abs(pos), wh/2-corner_radius + 1);

   vec2 diff = wh/2-corner_radius - abs(pos);
   float d2 = max(diff.x, diff.y);

   color_out.a = min(max(d1, 0.) + max(d2, 0.), 1.);
}
