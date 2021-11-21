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

   s._init = score.load
   s._draw = score.draw
   s._keypressed = score.keypressed
   s._update = score.update

   return s
end

return sokoban
