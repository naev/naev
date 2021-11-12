--[[
   Some sort of stellar wind type background.
--]]

-- We use the default background too!
local starfield = require "bkg.starfield"
local bgshaders = require "bkg.bgshaders"
local love_shaders = require 'love_shaders'

local shader, sf

function background ()
   -- Scale factor that controls computation cost. As this shader is really
   -- really expensive, we can't compute it at full resolution
   sf = naev.conf().nebu_scale

   -- Initialize shader
   shader = love_shaders.windy{ strength=sf, density=0.7 }
   bgshaders.init( shader, sf )

   -- Default nebula background
   starfield.init()
end

renderbg = starfield.render

function renderfg( dt )
   -- Get camera properties
   local x, y = camera.get():get()
   local z = camera.getZoom()
   shader:send( "u_camera", x*0.5/sf, -y*0.5/sf, z )

   bgshaders.render( dt, {0.2, 0.6, 0.9, 0.8} )
end
