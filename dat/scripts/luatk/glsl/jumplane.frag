#include "lib/sdf.glsl"

/* Based on dat/glsl/jumplane.frag */

uniform vec2 dimensions;
uniform float paramf;
uniform vec4 paramv;

vec4 effect( vec4 colour, Image tex, vec2 uv_in, vec2 px )
{
   vec2 uv        = (uv_in*2.0-1.0)*dimensions;
   float x        = uv_in.x*2.0-1.0;
   float d        = sdBox( uv, dimensions-vec2(1.0) );
   float alpha    = smoothstep( -1.0,  0.0, -d);

   vec4 colour_out;
   colour_out      = mix( colour, paramv, smoothstep(0.0,1.0,x*0.5+0.5) );
   colour_out.a   *= 0.8 - 0.6*abs(x);
   colour_out.a   *= smoothstep(dimensions.x, dimensions.x-paramf, length(uv));
   colour_out.a   *= alpha;
   return colour_out;
}
