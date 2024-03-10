uniform vec4 colour;
uniform sampler2D sampler;

in vec2 tex_coord;
out vec4 colour_out;

#if GLSL_VERSION >= 420
#define A_GPU 1
#define A_GLSL 1
#include "lib/ffx_a.h"
#define FSR_RCAS_F 1
#define FSR_RCAS_PASSTHROUGH_ALPHA 1
AU4 con0;
AF4 FsrRcasLoadF(ASU2 p) { return AF4(texelFetch(sampler, p, 0)); }
void FsrRcasInputF(inout AF1 r, inout AF1 g, inout AF1 b) {}
#include "lib/ffx_fsr1.h"
#endif /* GLSL_VERSION >= 420 */

void main()
{
#if GLSL_VERSION > 420
   FsrRcasCon(con0, 0.0); /* Max sharpening. */
   AU2 gxy = AU2( tex_coord.xy * textureSize(sampler, 0) );
   FsrRcasF( colour_out.r, colour_out.g, colour_out.b, colour_out.a, gxy, con0 );
   colour_out *= colour;

#else /* GLSL_VERSION > 420 */
   /* mpv's unsharpen mask. */
   const float PARAM = 0.5; /**< Sharpening strength. */
   vec2 d = vec2(1.0) / textureSize( sampler, 0 );
   float st1 = 1.2;
   vec4 p = texture( sampler, tex_coord );
   vec4 sum1 = texture( sampler, st1 * vec2(+d.x, +d.yy) )
      + texture( sampler, st1 * vec2(+d.x, -d.y))
      + texture( sampler, st1 * vec2(-d.x, +d.y))
      + texture( sampler, st1 * vec2(-d.x, -d.y));
   float st2 = 1.5;
   vec4 sum2 = texture( sampler, st2 * vec2(+d.x,  0.0))
      + texture( sampler, st2 * vec2( 0.0, +d.y))
      + texture( sampler, st2 * vec2(-d.x,  0.0))
      + texture( sampler, st2 * vec2( 0.0, -d.y));
   vec4 t = p * 0.859375 + sum2 * -0.1171875 + sum1 * -0.09765625;
   colour_out = colour * (p + t * PARAM);
#endif /* GLSL_VERSION > 420 */
}
