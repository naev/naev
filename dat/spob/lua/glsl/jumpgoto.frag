#include "lib/sdf.glsl"

/* Based on dat/glsl/jumplanegoto.frag */

uniform vec2 dimensions;
uniform float paramf = 0.0;
uniform float dt;

vec4 effect( vec4 colour, Image tex, vec2 uv_in, vec2 px )
{
   vec2 uv = (uv_in*2.0-1.0)*dimensions;
   float m = 1.0 / dimensions.x;
   float d = sdBox( uv, dimensions-vec2(1.0) );

   vec2 uvs = uv;
   uvs.y  = abs(uvs.y);
   uvs.x -= dt*dimensions.y*0.8;
   uvs.x  = mod(-uvs.x,dimensions.y*1.5)-0.25*dimensions.y;
   float ds = -0.2*abs(uvs.x-0.5*uvs.y) + 2.0/3.0;
   d = max( d, ds );

   float alpha    = smoothstep(-1.0, 0.0, -d);
   vec4 colour_out= colour;
   colour_out.a  *= alpha;
   colour_out.a  *= smoothstep(dimensions.x, dimensions.x-paramf, length(uv));
   return colour_out;
}
