#include "lib/simplex.glsl"
#include "lib/blend.glsl"
#include "lib/blur.glsl"

uniform vec3 dimensions;
uniform sampler2D u_tex;

uniform float u_r;
uniform float u_timer;
uniform float u_elapsed;

const float TIME_TOTAL  = 3.0;
const float AMPLITUDE   = 30.0;
const float SPEED       = 30.0;
const float STRENGTH    = 0.2;
const vec3 GLOW_COL     = vec3(1.0);

in vec2 tex_coord;
in vec2 tex_scale;
out vec4 colour_out;

void main(void) {
   vec2 uv = tex_coord / tex_scale;
   vec2 dir = uv - vec2(0.5);
   float dist = length(dir) * dimensions.z;
   float progress = TIMER / TIME_TOTAL;

   vec2 off = progress * dir * sin( dist * AMPLITUDE - progress * SPEED );
   colour_out = texture( u_tex, (uv + STRENGTH*off) * tex_scale );
   colour_out.rgb += progress * GLOW_COL;
   if (progress > 0.5)
      colour_out.a   *= 2.0*(1.0-progress);
}
