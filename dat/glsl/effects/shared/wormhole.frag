#include "lib/simplex.glsl"
#include "lib/blend.glsl"
#include "lib/blur.glsl"

/* This is not meant for use directly. Please #define TIMER to be either
u_elapsed or u_timer and include this file. */

uniform vec3 dimensions;
uniform sampler2D u_tex;

uniform float u_r;
uniform float u_timer;
uniform float u_elapsed;

const float TIME_GLOW   = 1.1;
const float TIME_TOTAL  = 3.0;
const vec3 GLOW_COL     = vec3( 0.1, 0.8, 1.0 );

in vec2 tex_coord;
out vec4 colour_out;

void main(void) {
   vec2 uv = tex_coord;
   float blur = 3.0;

   colour_out = texture( u_tex, uv );

   if (TIMER > TIME_GLOW)
      blur *= 1.0 + (TIMER-TIME_GLOW) * 3.0;

   float glow = blur13( u_tex, uv, dimensions.xy, blur ).a;
   glow = 3.0*glow;

   if (TIMER < 1.0 )
      glow *= TIMER;

   colour_out.a  += glow;
   colour_out.rgb += min(colour_out.a, glow) * GLOW_COL;

   if (TIMER > TIME_GLOW) {
      vec2 coord = 0.05 * uv * dimensions.xy / dimensions.z + u_r;
      float n = 0.5 + 0.5 * snoise( coord );

      colour_out.a *= min( 1.0, 1.0 + (TIME_TOTAL-TIME_GLOW) * (n - (TIMER-TIME_GLOW))  );
   }
}
