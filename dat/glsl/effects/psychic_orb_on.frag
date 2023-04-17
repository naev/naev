#include "lib/simplex.glsl"
#include "lib/sdf.glsl"

uniform vec3 dimensions;
uniform sampler2D u_tex;

uniform float u_r       = 0.0;
//uniform float u_timer   = 0.0;
uniform float u_elapsed = 0.0;
uniform float u_dir     = 0.0;

in vec2 tex_coord;
in vec2 tex_scale;
out vec4 colour_out;

void main(void)
{
   vec2 st = tex_coord;
   vec2 uv = 2.0 * tex_coord / tex_scale - 1.0;

   colour_out = vec4( 1.0, 0.8, 0.0, 1.0 );

   float d = sdCircle( uv, 0.7+0.1*sin(u_elapsed) );
   vec3 nuv = vec3(3.0 * uv, u_elapsed) + vec3(u_r);
   float n = 0.3*snoise( 1.0 * nuv );
   colour_out.a *= smoothstep( -0.2, 0.2, -d );

   colour_out   += 0.6 * n * smoothstep( -0.1, 0.1, -d );
}
