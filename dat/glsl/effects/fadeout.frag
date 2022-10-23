#include "lib/simplex.glsl"
uniform sampler2D u_tex;

uniform float u_timer   = 0.0;
uniform float u_elapsed = 0.0;

const float LENGTH   = 3.0;

in vec2 tex_coord;
out vec4 colour_out;

void main(void) {
   vec2 uv = tex_coord;

   /* Real texture. */
   colour_out = texture( u_tex, uv );

   if (u_timer < LENGTH*0.5) {
      colour_out.rgb = vec3(0.0);
      colour_out.a *= u_timer / (LENGTH*0.5);
   }
   else {
      float progress = (u_timer - LENGTH*0.5) / (LENGTH*0.5);
      colour_out.rgb = mix( vec3(0.0), colour_out.rgb, progress );
   }
}
