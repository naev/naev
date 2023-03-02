#include "lib/simplex.glsl"
#include "lib/blur.glsl"

uniform vec3 dimensions;
uniform sampler2D u_tex;

uniform float u_r       = 0.0;
//uniform float u_timer   = 0.0;
uniform float u_elapsed = 0.0;
uniform float u_dir     = 0.0;

const vec4 COLOUR_OUTLINE  = vec4( 0.9, 0.9, 0.4, 3.0/3.0 );
const vec4 COLOUR_BRIGHT   = vec4( 0.6, 0.6, 0.3, 2.0/3.0 );
const vec4 COLOUR_DARK     = vec4( 0.5, 0.7, 0.4, 1.0/3.0 );

in vec2 tex_coord;
in vec2 tex_scale;
out vec4 colour_out;

void main(void)
{
   vec2 st = tex_coord;
   vec2 uv = 2.0 * tex_coord / tex_scale - 1.0;
   float c = cos(u_dir);
   float s = sin(u_dir);
   mat2 R = mat2( c, -s, s, c );
   vec2 nuv = 3.0 * (uv + u_r) + vec2( u_elapsed, 0.0 );
   float n;
   n  = 0.625 * snoise( nuv );
   n += 0.250 * snoise( nuv*2.0 );
   n += 0.125 * snoise( nuv*4.0 );
   st += tex_scale.x * 0.1 * n * vec2( c, s );
   //st += tex_scale.x * 0.1 * sin(u_elapsed) * vec2( c, s );

   vec4 base_colour = texture( u_tex, st );
   if (base_colour.a <= 0.0)
      discard;

   float w    = length(base_colour.rgb);
   float m    = abs(1.0-2.0*fract(u_elapsed+10.0*((st.x+0.02*cos(64.0*st.y)))));
   float f    = 0.5+0.75*abs(1.0-2.0*fract((1.7+0.03*st.x)*u_elapsed+abs(1.0-2.0*fract((1.9+0.03*st.y)*u_elapsed))));
   colour_out = clamp(mix(f*COLOUR_OUTLINE, mix(COLOUR_DARK, COLOUR_BRIGHT, m*(m+0.5)), w), 0.0, 1.0);
   colour_out *= base_colour.a;
}
