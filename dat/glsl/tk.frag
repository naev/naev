uniform vec4 dc;
uniform vec4 c;
uniform vec4 lc;
uniform vec4 oc;
uniform vec2 wh;
uniform float corner_radius;

in float ratio;
in vec2 pos;
out vec4 color_out;

void main(void) {
   vec2 diff = wh/2 - abs(pos);

   // Distance from rectangle edge
   float d1 = min(diff.x, diff.y);

   // Distance from corner circle edge
   float d2 = corner_radius - distance(abs(pos), wh/2 - corner_radius);

   // Distance from rounded rectangle edge
   float d = mix(d2, d1, step(corner_radius, max(diff.x, diff.y)));

   // Gradiant body
   color_out = mix(dc, c, ratio);
   // Inner border
   color_out = mix(lc, color_out, clamp(d - 2, 0., 1.));
   // Outer border
   color_out = mix(oc, color_out, clamp(d - 1, 0., 1.));

   color_out.a = d;
}
