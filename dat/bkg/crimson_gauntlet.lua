--[[
   Crimson Gauntlet, Digital Battleground

   Some light circuit background.
--]]

-- We use the default background too!
require "bkg.default"

local love = require 'love'
local lg = require 'love.graphics'
local love_shaders = require 'love_shaders'

-- Since we don't actually activate the Love framework we have to fake the
-- the dimensions and width, and set up the origins.
local nw, nh = naev.gfx.dim()
love.x = 0
love.y = 0
love.w = nw
love.h = nh
lg.origin()

function background ()
   w, h = naev.gfx.dim()
   shader = love_shaders.circuit{}

   -- Default nebula background (no star)
   cur_sys = system.cur()
   prng:setSeed( cur_sys:name() )
   background_nebula()
end

function renderbg( dt )
   -- Get camera properties
   --local x, y = camera.get():get()
   local z = camera.getZoom()
   shader:send( "u_camera", 0, 0, z )
   shader:update( dt )

   local b = 0.1
   lg.setColor( b, b, b, 1 )
   local oldshader = lg.getShader()
   lg.setShader( shader )
   love_shaders.img:draw( 0, 0, 0, w, h )
   lg.setShader( oldshader )
end
