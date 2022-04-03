--[[
   Active hypergate
--]]
local lg = require "love.graphics"
local love_shaders = require "love_shaders"

local pos, tex, mask, cvs, shader
local tw, th

local pixelcode = [[
#include "lib/simplex.glsl"

uniform float u_time = 0.0;
const vec3 basecol = vec3( 0.2, 0.8, 0.8 );

float fbm3( vec2 x )
{
   float v = 0.0;
   float a = 0.5;
   const vec2 shift = vec2(100.0);
   for (int i=0; i<3; i++) {
      v += a * snoise(x);
      x  = x * 2.0 + shift;
      a *= 0.5;
   }
   return v;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec2 centered = texture_coords*2.0-1.0;
   vec2 polar = vec2( length(centered), atan( centered.y, centered.x ) );
   polar.x -= u_time * 0.01;

   vec4 mask = texture( tex, texture_coords );
   vec4 col = vec4( basecol, 1.0 );

   float noise = fbm3( vec2( fbm3( centered ), fbm3( polar ) ) );

   //col.a *= (0.6 + 0.4*snoise( polar * 3.0 ));
   col.a *= (0.6 + 0.4 * noise);
   col.a *= mask.r;
   return col;
}
]]

local function update_canvas ()
   local oldcanvas = lg.getCanvas()
   lg.setCanvas( cvs )
   lg.clear( 0, 0, 0, 0 )
   lg.setColor( 1, 1, 1, 1 )
   --lg.setBlendMode( "alpha", "premultiplied" )

   -- Draw base hypergate
   tex:draw( 0, 0 )

   -- Draw active overlay shader
   local oldshader = lg.getShader()
   lg.setShader( shader )
   mask:draw( 0, 0 )
   lg.setShader( oldshader )

   --lg.setBlendMode( "alpha" )
   lg.setCanvas( oldcanvas )
end

function load( p )
   if tex==nil then
      -- Set up texture stuff
      local prefix = "gfx/spob/space/"
      tex  = lg.newImage( prefix.."hypergate_neutral_activated.webp" )
      mask = lg.newImage( prefix.."hypergate_mask.webp" )

      -- Position stuff
      pos = p:pos()
      tw, th = tex:getDimensions()
      pos = pos + vec2.new( -tw/2, th/2 )

      -- The canvas
      cvs  = lg.newCanvas( tw, th, {dpiscale=1} )

      -- Set up shader
      shader = lg.newShader( pixelcode, love_shaders.vertexcode )
      shader._dt = -1000 * rnd.rnd()
      shader.update = function( self, dt )
         self._dt = self._dt + dt
         self:send( "u_time", self._dt )
      end

      update_canvas()
   end

   return cvs.t.tex, tw/2
end

function unload ()
   shader= nil
   tex   = nil
   mask  = nil
   cvs   = nil
   --sfx   = nil
end

function render ()
   update_canvas() -- We want to do this here or it gets slow in autonav
   local z = camera.getZoom()
   local x, y = gfx.screencoords( pos, true ):get()
   z = 1/z
   cvs:draw( x, y, 0, z, z )
end

function update( dt )
   shader:update( dt )
end

function can_land ()
   return false
end
