#include "lib/perlin.glsl"
#include "lib/colour.glsl"
#include "lib/nebula.glsl"

layout(std140) uniform NebulaData {
   float hue;
   float horizon;
   float eddy_scale;
   float elapsed;
   float nonuniformity;
   float volatility;
   float saturation;
   vec2 camera;
};

in vec4 base_col;
layout(location = 0) out vec4 colour_out;

void main (void)
{
   float dist, f, hhue;
   vec3 uv;
   vec4 colour;

   /* Compute coordinates for the noise */
   vec2 rel_pos = gl_FragCoord.xy - camera;
   dist = length(rel_pos);
   if (dist > 2.0*horizon) {
      colour_out = base_col;
      return;
   }

   uv.xy = rel_pos / eddy_scale;
   uv.z = elapsed * 0.5;

   /* Compute hue as in lib/nebula.glsl. */
   hhue = nebula_hue( hue, uv );
   colour = base_col;

   /* Modify coordinates to be larger and slower. */
   uv.xy = 3.0 * uv.xy + 1000.0; // Scaled/offset from nebula_background
   uv.z *= 1.5;

   /* Compute dist and interpolate */
   colour_out = colour;
   colour_out.a *= smoothstep( 0.0, horizon, dist );
}
