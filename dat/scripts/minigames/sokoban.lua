--[[
   Wrapper for the Sokoban minigame
--]]
local love = require "love"
local vn = require "vn"
local score = require "minigames.sokoban.core"
local sokoban = {}

--[[
   Runs the Sokoban minigame as a standalone
--]]
function sokoban.love ()
   love.exec( 'scripts/minigames/sokoban' )
end

--[[
   Runs the Sokoban minigame from the VN
--]]
function sokoban.vn ()
   local s = vn.custom()

   s._init = function ()
      return score.load()
   end
   s._draw = function ()
      return score.draw()
   end
   s._keypressed = function( _self, key )
      return score.keypressed( key )
   end
   s._update = function( _self, dt )
      return score.update( dt )
   end

   return s
end

return sokoban
