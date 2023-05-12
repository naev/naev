--[[This file contains the attack profiles by ship type.
--commonly used range and condition-based attack patterns are found in another file
--Think functions for determining who to attack are found in another file
--]]

local atk = require "ai.core.attack.util"

local atk_fighter = {}

--[[
-- Mainly targets small fighters.
--]]
function atk_fighter.think( target, _si )
   -- Lower chance to not change targets
   if rnd.rnd() < 0.3 then
      return
   end

   local enemy = atk.preferred_enemy()
   local dist  = ai.dist( target )
   local range = ai.getweaprange(3, 0)

   -- Get new target if it's closer
   --prioritize targets within the size limit
   if enemy ~= target and enemy ~= nil then
      -- Shouldn't switch targets if close
      if dist > range * mem.atk_changetarget then
         ai.pushtask("attack", enemy )
      end
      return
   end

   local nearest_enemy = ai.getenemy()
   if nearest_enemy ~= target and nearest_enemy ~= nil then
      -- Shouldn't switch targets if close
      if dist > range * mem.atk_changetarget then
         ai.pushtask("attack", nearest_enemy )
      end
   end
end


--[[
-- Main control function for fighter behavior.
--]]
function atk_fighter.atk( target, dokill )
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

   -- We first bias towards range
   if dist > range * mem.atk_approach and mem.ranged_ammo > mem.atk_minammo then
      atk.ranged( target, dist ) -- Use generic ranged function

   -- Otherwise melee
   else
      if target:stats().mass < 200 then
         atk.space_sup( target, dist )
      else
         atk.flyby( target, dist )
      end
   end
end


return atk_fighter
