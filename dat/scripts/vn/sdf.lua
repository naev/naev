local love_shaders = require "love_shaders"
local graphics = require 'love.graphics'

local sdf = {}

sdf.arrow = graphics.newShader( [[
#include "lib/sdf.glsl"
vec4 effect( vec4 colour, Image tex, vec2 pos, vec2 px )
{
   vec2 uv = pos*2.0-1.0;
   uv = vec2(-uv.y,-uv.x);

   float d1 = sdTriangleIsosceles( uv+vec2(0.0,-0.4), vec2(0.6,0.4) );
   float d2 = sdTriangleIsosceles( uv+vec2(0.0,0.2),  vec2(0.6,0.4) );
   float d3 = sdTriangleIsosceles( uv+vec2(0.0,0.8),  vec2(0.6,0.4) );

   float d = min(min(d1, d2), d3);

   d = abs(d)-0.01;

   colour.a *= step( 0.0, -d ) + pow( 1.0-d, 20.0 );
   return colour;
}
]], love_shaders.vertexcode)

sdf.gear = graphics.newShader( [[
#include "lib/sdf.glsl"
#include "lib/math.glsl"

vec4 effect( vec4 colour, Image tex, vec2 pos, vec2 px )
{
   vec2 uv = vec2( pos.y, pos.x )*2.0-1.0;
   float m = 1.0 / love_ScreenSize.x;

   float d = sdCircle( uv, 0.75 );

   vec2 auv = abs(uv);
   if (auv.y < auv.x)
      auv.xy = vec2( auv.y, auv.x );
   const float s = sin( -M_PI/8.0 );
   const float c = cos( -M_PI/8.0 );
   const mat2 R = mat2( c, s, -s, c );
   d = sdSmoothUnion( d, sdBox( auv*R, vec2(0.1,1.0-m) )-0.025, 0.2 );

   d = max( d, -sdCircle( uv, 0.6 ) );
   d = min( d, sdCircle( uv, 0.25 ) );

   float alpha = smoothstep(-m, 0.0, -d);
   float beta  = smoothstep(-2.0*m, -m, -d);
   return colour * vec4( vec3(alpha), beta );
}
]], love_shaders.vertexcode)

sdf.img = love_shaders.img

return sdf
