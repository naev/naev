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

return sdf
