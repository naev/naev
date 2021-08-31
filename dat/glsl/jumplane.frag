#include "lib/sdf.glsl"

uniform vec4 color;
uniform vec2 dimensions;
uniform float paramf;
uniform vec4 paramv;

in vec2 pos;
out vec4 color_out;

void main(void) {
   vec2 uv        = pos * dimensions;
   float d        = sdBox( uv, dimensions-vec2(1.0) );
   float alpha    = smoothstep( -1.0,  0.0, -d);
   color_out      = mix( color, paramv, smoothstep(0.0,1.0,pos.x*0.5+0.5) );
   color_out.a   *= 0.8 - 0.6*abs(pos.x);
   color_out.a   *= smoothstep(dimensions.x, dimensions.x-paramf, length(uv));
   color_out.a   *= alpha;
}
