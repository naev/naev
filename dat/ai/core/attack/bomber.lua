local atk = require "ai.core.attack.util"

function atk_bomber_init ()
   mem.atk_think  = atk_heuristic_big_game_think
   mem.atk        = atk_bomber
end


--[[
-- Main control function for bomber behavior.
-- Bombers are expected to have heavy weapons and target
--ships bigger than they are
--]]
function atk_bomber( target, dokill )
   target = atk.com_think( target, dokill )
   if target == nil then return end

   -- Targeting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- See if the enemy is still seeable
   if not atk.check_seeable( target ) then return end

   -- Get stats about enemy
   local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange(3, 0)

   -- TODO bombers need their own specific routines
   if dist > range * mem.atk_approach and mem.ranged_ammo > mem.atk_minammo then
      __atk_g_ranged( target, dist )

   else
      __atk_f_flyby( target, dist )
   end
end

