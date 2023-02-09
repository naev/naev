uniform mat4 projection;
uniform vec3 dims;
uniform vec3 screen;
uniform vec2 offset_xy;

in vec4 vertex;
in float brightness;
out float brightness_geom;
out vec2 radius_geom;

void main(void) {
   vec4 center = vertex;
   float b = 1.0/(9.0 - 10.0*brightness);
   center.xy += offset_xy * b;
   center.xy = mod(center.xy + screen.xy/2.0, screen.xy) - screen.xy/2.0;

   radius_geom = dims.x * vec2( projection[0][0], projection[1][1] );

   /* Calculate position */
   gl_Position = projection * center;
   brightness_geom = brightness;
}
