--[[
   Wrapper for the String guessing minigame
--]]
local love = require "love"
local vn = require "vn"
local score = require "minigames.flip.core"
local flip = {}

local function setup( params, standalone )
   params = params or {}
   local c = naev.cache()
   c.flip = {}
   c.flip.params = params
   c.flip.standalone = standalone
   c.flip.won = false
end

--[[
   Runs the Sokoban minigame as a standalone
--]]
function flip.love( params )
   setup( params, true )
   love.exec( 'scripts/minigames/flip' )
end

--[[
   Runs the Sokoban minigame from the VN
--]]
function flip.vn( params )
   local s = vn.custom()
   s._init = function ()
      setup( params, false )
      return score.load()
   end
   s._draw = function ()
      naev.gfx.clearDepth()
      return score.draw()
   end
   s._keypressed = function( _self, key )
      return score.keypressed( key )
   end
   s._mousepressed = function( _self, mx, my, button )
      return score.mousepressed( mx, my, button )
   end
   s._update = function( self, dt )
      if score.update( dt ) then
         self.done = true
      end
   end
   return s
end

function flip.completed ()
   local c = naev.cache()
   return (c.flip and c.flip.won)
end

return flip
