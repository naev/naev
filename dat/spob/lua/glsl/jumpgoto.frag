#pragma language glsl3

#include "lib/sdf.glsl"

uniform vec2 dimensions;
uniform float paramf = 0.0;
uniform float dt;

vec4 effect( vec4 colour, Image tex, vec2 uv_in, vec2 px )
{
   /*
   uv = uv*2.0-1.0;
   float d = abs(length(uv)-0.9)-0.1;
   float m = 1.0 / radius;

   float a = atan( uv.y, uv.x );
   if ((a < 0.0) && (pointer > M_PI))
      a += 2.0*M_PI;
   if (pointer < a)
      d = 1.0;
   else
      colour.a *= 1.0-0.8*pow(pointer-a,0.5);

   float alpha = smoothstep( -m, 0.0, -d);
	return colour * vec4( vec3(1.0), alpha );
   */
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
