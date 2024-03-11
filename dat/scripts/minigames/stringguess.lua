--[[
   Wrapper for the String guessing minigame
--]]
local love = require "love"
local vn = require "vn"
local score = require "minigames.stringguess.core"
local stringguess = {}

local function setup( params, standalone )
   params = params or {}
   local c = naev.cache()
   c.stringguess = {}
   c.stringguess.params = params
   c.stringguess.standalone = standalone
   c.stringguess.won = false
end

--[[
   Runs the Sokoban minigame as a standalone
--]]
function stringguess.love( params )
   setup( params, true )
   love.exec( 'scripts/minigames/stringguess' )
end

--[[
   Runs the Sokoban minigame from the VN
--]]
function stringguess.vn( params )
   local s = vn.custom()
   s._init = function ()
      setup( params, false )
      return score.load()
   end
   s._draw = function ()
      naev.gfx.clearDepth()
      return score.draw()
   end
   s._mousemoved = function( _self, mx, my )
      return score.mousemoved( mx, my )
   end
   s._mousepressed = function( _self, mx, my )
      return score.mousepressed( mx, my )
   end
   s._keypressed = function( _self, key )
      return score.keypressed( key )
   end
   s._update = function( self, dt )
      if score.update( dt ) then
         self.done = true
      end
   end
   return s
end

function stringguess.completed ()
   local c = naev.cache()
   return (c.stringguess and c.stringguess.won)
end

return stringguess
