#include "lib/sobel.glsl"

uniform vec3 dimensions;
uniform sampler2D u_tex;

uniform float u_r       = 0.0;
uniform float u_timer   = 0.0;
uniform float u_elapsed = 0.0;

const float FADE  = 3.0;
//const vec3 COL    = vec3(0.8, 0.0, 0.7);
const vec3 COL    = vec3(0.0, 0.7, 0.8);

in vec2 tex_coord;
out vec4 colour_out;

void main(void) {
   colour_out = texture( u_tex, tex_coord );
   if (colour_out.a <= 0.0)
      discard;

   float fade = min( 1.0, min( u_elapsed, u_timer )*FADE );
   colour_out.rgb = mix( vec3(0.0), COL, fade*sobel( u_tex, tex_coord, dimensions.xy ) );
}
