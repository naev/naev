#include "lib/sdf.glsl"

uniform vec4 colour;
uniform vec2 dimensions;
uniform float paramf;
uniform int parami;
uniform float dt;

in vec2 pos;
out vec4 colour_out;

void main(void) {
   vec2 uv = pos*dimensions;
   float m = 1.0 / dimensions.x;
   float d = sdBox( uv, dimensions-vec2(1.0) );

   if (parami != 0) {
      vec2 uvs = uv;
      uvs.y  = abs(uvs.y);
      uvs.x -= dt*dimensions.y*0.8;
      uvs.x  = mod(-uvs.x,dimensions.y*1.5)-0.25*dimensions.y;
      float ds = -0.2*abs(uvs.x-0.5*uvs.y) + 2.0/3.0;
      d = max( d, ds );
   }

   float alpha    = smoothstep(-1.0, 0.0, -d);
   colour_out      = colour;
   colour_out.a   *= alpha;
   colour_out.a   *= smoothstep(dimensions.x, dimensions.x-paramf, length(uv));
}
