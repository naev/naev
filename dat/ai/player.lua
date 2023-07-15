--[[
   AI for the player, basically meant for escorts and other functionality.
--]]
require 'ai.core.core'
mem.aggressive = true -- Player is an asshole
mem.atk_kill = false -- Should propagate to fighters
mem.lanes_useneutral = true -- Uses neutral lanes

function should_attack( enemy, _si, _aggressor )
   if not enemy or not enemy:exists() then
      return false
   end
   return true -- should work better with escorts
end
