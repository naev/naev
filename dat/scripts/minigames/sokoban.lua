--[[
   Wrapper for the Sokoban minigame
--]]
local love = require "love"
local vn = require "vn"
local score = require "minigames.sokoban.core"
local sokoban = {}

local function setup( params, standalone )
   params = params or {}
   local c = naev.cache()
   c.sokoban = {}
   c.sokoban.params = params
   c.sokoban.standalone = standalone
end

--[[
   Runs the Sokoban minigame as a standalone
--]]
function sokoban.love( params )
   setup( params, true )
   love.exec( 'scripts/minigames/sokoban' )
end

--[[
   Runs the Sokoban minigame from the VN
--]]
function sokoban.vn( params )
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
   s._update = function( self, dt )
      if score.update( dt ) then
         self.done = true
      end
   end
   return s
end

function sokoban.completed ()
   local c = naev.cache()
   return (c.sokoban and c.sokoban.completed)
end

return sokoban
