--[[
   Some sort of solar wind type background.
--]]

-- We use the default background too!
require "bkg.default"
local bgshaders = require "bkg.bgshaders"
local love_shaders = require 'love_shaders'

function background ()
   -- Scale factor that controls computation cost. As this shader is really
   -- really expensive, we can't compute it at full resolution
   sf = naev.conf().nebu_scale

   -- Initialize shader
   shader = love_shaders.windy{ strength=sf }
   bgshaders.init( shader, sf )

   -- Default nebula background
   cur_sys = system.cur()
   prng:setSeed( cur_sys:name() )
   background_nebula()
   background_stars()
end

function renderfg( dt )
   -- Get camera properties
   local x, y = camera.get():get()
   local z = camera.getZoom()
   shader:send( "u_camera", x*0.5/sf, -y*0.5/sf, z )

   bgshaders.render( dt, {0.2, 0.6, 0.9, 0.8} )
end
