#include "lib/simplex.glsl"
#include "lib/blend.glsl"
#include "lib/blur.glsl"

uniform vec3 dimensions;
uniform sampler2D u_tex;

uniform float u_r       = 0.0;
uniform float u_timer   = 0.0;
uniform float u_elapsed = 0.0;

const vec3 GLOW_COL     = vec3(1.0);

in vec2 tex_coord;
out vec4 colour_out;

void main(void) {
   colour_out = texture( u_tex, tex_coord );

   float glow = blur9( u_tex, tex_coord, dimensions.xy, 3.0 ).a * 5.0;

   if (u_elapsed < 1.0 )
      glow *= u_elapsed;

   colour_out.rgb = blendGlow( colour_out.rgb, GLOW_COL, glow );
   colour_out.a  += glow;

   if (u_elapsed > 1.1) {
      vec2 coord = 0.05 * tex_coord * dimensions.xy / dimensions.z + u_r;
      float n = 0.5 + 0.5 * snoise( coord );

      colour_out.a *= min( 1.0, 1.0 + n - 1.9 * (u_elapsed-1.1)  );
   }
}
