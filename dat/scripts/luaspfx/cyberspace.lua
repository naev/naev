local lf = require 'love.filesystem'
local lg = require 'love.graphics'
--local audio = require 'love.audio'
--local pp_shaders = require "pp_shaders"
local love_image = require "love.image"

local cyberspace_shader, img

local function update( sp, dt )
   local d = sp:data()
   d.str = math.min( 1.0, d.str+dt )
end

local function render( sp, x, y, z )
   local d = sp:data()
   local w,h = gfx.dim()

   cyberspace_shader:send( "u_str", d.str )
   cyberspace_shader:send( "u_pos", {-x,-y,z} )

   lg.setColour( 0.8, 0, 0.7 )
   lg.setShader( cyberspace_shader )
   img:draw( 0, 0, 0, w, h )
   lg.setShader()
end

local function cyberspace( _params )
   --params = params or {}
   -- Lazy loading shader / sound
   if not cyberspace_shader then
      cyberspace_shader = lg.newShader( lf.read( "scripts/luaspfx/shaders/cyberspace.frag" ), nil )
      local idata = love_image.newImageData( 1, 1 )
      idata:setPixel( 0, 0, 1, 1, 1, 1 )
      img = lg.newImage( idata )
   end

   -- Sound is handled separately in outfit
   local s = spfx.new( math.huge, update, render, nil, nil, nil, nil, nil, -1 )
   local d  = s:data()
   d.str = 0;
   return s
end

return cyberspace
