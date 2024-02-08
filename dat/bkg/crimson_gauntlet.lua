--[[
   Crimson Gauntlet, Digital Battleground

   Some light circuit background.
--]]

-- We use the default background too!
local starfield = require "bkg.lib.starfield"
local bgshaders = require "bkg.lib.bgshaders"
local love_shaders = require 'love_shaders'
local nebula = require "bkg.lib.nebula"

local shader, scircuit

function background ()
   -- Shader isn't too expensive, but we try to respect nebu_scale to an
   -- extent.
   local sf = math.max( 1, naev.conf().nebu_scale / 2 )

   -- Initialize the shader
   shader = love_shaders.circuit{ strength=sf }
   scircuit = bgshaders.init( shader, sf )

   -- Default nebula background (no star)
   starfield.init{ nolocalstars = true }

   -- Let's add big nebula because we can
   nebula.init{ size=3000, movemod=0.2, opacity=50 }

   -- Slighty blue
   gfx.lightAmbient( 0.6, 0.8, 1.0, 0.3 )
end

renderbg = starfield.render

function renderfg( dt )
   -- Get camera properties
   --local x, y = camera.pos():get()
   local z = camera.getZoom()
   shader:send( "u_camera", 0, 0, z )

   scircuit:render( dt, {1, 1, 1, 0.02} )
end
