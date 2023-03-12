#include "lib/simplex.glsl"
#include "lib/blend.glsl"

uniform vec3 dimensions;
uniform sampler2D u_tex;

uniform float u_r       = 0.0;
uniform float u_timer   = 0.0;
uniform float u_elapsed = 0.0;
uniform float u_dir     = 0.0;

const float FADE  = 0.3;
const vec4 COLOUR = vec4( 1.0, 0.8, 0.0, 1.0 );

in vec2 tex_coord;
in vec2 tex_scale;
out vec4 colour_out;

void main(void)
{
   vec2 st = tex_coord; // a bit oversized
   vec2 uv = 2.0 * tex_coord / tex_scale - 1.0;
   float c = cos(u_dir+M_PI);
   float s = sin(u_dir+M_PI);
   mat2 R = mat2( c, -s, s, c );
   vec2 uvr = R*uv;
   vec2 nuv = 3.0 * (uvr + u_r ) + vec2(u_elapsed, 0.0);
   float n;
   n  = 0.625 * snoise( nuv );
   n += 0.375 * snoise( nuv*2.0 );
   //n += 0.125 * snoise( nuv*4.0 );
   st += tex_scale.x * 0.3 * n * vec2( c, s );
   vec4 colour_overlay = texture( u_tex, st );

   /* Base colour. */
   colour_out = texture( u_tex, tex_coord );

   /* Filter transparent pixels. */
   if (max( colour_overlay.a, colour_out.a ) <= 0.0)
      discard;

   nuv = 0.7*nuv + 100.0;
   n  = 0.625*snoise(nuv);
   n += 0.375*snoise(nuv*2.0);
   float alpha = 0.2 * colour_overlay.a * (0.6 + 0.4*n);
   alpha *= smoothstep( 0.0, FADE, u_timer );
   alpha *= smoothstep( 0.0, FADE, u_elapsed );
   colour_out.rgb = blendGlow( colour_out.rgb, COLOUR.rgb, alpha );
   colour_out.a = max( alpha, colour_out.a );
}
