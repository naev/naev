--[[
   Some sort of stellar wind type background.
--]]
local bgshaders = require "bkg.lib.bgshaders"
local love_shaders = require 'love_shaders'
-- We use the default background too!
require "bkg.default"

local shader, sf, swind

local background_default = background
function background ()
   -- Scale factor that controls computation cost. As this shader is really
   -- really expensive, we can't compute it at full resolution
   sf = naev.conf().nebu_scale

   -- Initialize shader
   shader = love_shaders.windy{ strength=sf, density=0.7 }
   swind = bgshaders.init( shader, sf )

   -- Default nebula background
   background_default()

   -- Ambient light is coloured now
   gfx.lightAmbient( 0.2, 0.6, 0.9, 0.2, 1.5 )
   gfx.lightIntensity( 0.8 )
end

function renderfg( dt )
   -- Get camera properties
   local x, y, z = camera.get()
   local m = 0.5
   shader:send( "u_camera", x*m/sf, -y*m/sf, (1-m)+m*z )

   swind:render( dt, {0.2, 0.6, 0.9, 0.8} )
end
