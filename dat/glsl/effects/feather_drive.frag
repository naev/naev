#include "lib/simplex.glsl"
#include "lib/blend.glsl"
#include "lib/blur.glsl"

uniform vec3 dimensions;
uniform sampler2D u_tex;

//uniform float u_r       = 0.0;
uniform float u_timer;
uniform float u_elapsed;

const float LENGTH = 0.3;
const vec3 FADECOL = vec3( 1.0, 0.8, 0.0 );

in vec2 tex_coord;
in vec2 tex_scale;
out vec4 colour_out;

void main(void)
{
   float progress = u_elapsed / LENGTH;
   vec2 uv = tex_coord / tex_scale;
   float d = length( uv*2.0-1.0 ) - 2.0*progress;

   if (d > 0.0)
      discard;

   colour_out = texture( u_tex, tex_coord );
   colour_out.rgb += max( vec3(0.0), FADECOL * (d + 1.0) );
}
