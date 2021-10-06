--[[
   Crimson Gauntlet, Digital Battleground

   Some light circuit background.
--]]

-- We use the default background too!
require "bkg.default"
local bgshaders = require "bkg.bgshaders"

local love_shaders = require 'love_shaders'

function background ()
   -- Shader isn't too expensive, but we try to respect nebu_scale to an
   -- extent.
   sf = math.max( 1, naev.conf().nebu_scale / 2 )

   -- Initialize the shader
   shader = love_shaders.circuit{ strength=sf }
   bgshaders.init( shader, sf )

   -- Default nebula background (no star)
   cur_sys = system.cur()
   prng:setSeed( cur_sys:name() )
   background_nebula()
end

function renderfg( dt )
   -- Get camera properties
   --local x, y = camera.get():get()
   local z = camera.getZoom()
   shader:send( "u_camera", 0, 0, z )

   bgshaders.render( dt, {1, 1, 1, 0.02} )
end
