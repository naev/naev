--[[
--    Generic attack functions
--]]
local atk = require "ai.core.attack.util"

local atk_generic = {}

--[[
-- Mainly manages targeting nearest enemy.
--]]
function atk_generic.think( target, _si )
   -- A chance to just focus on the current enemy
   if rnd.rnd() < 0.5 then
      return
   end

   -- Get new target if it is better
   local enemy = atk.preferred_enemy()
   if enemy ~= target and enemy ~= nil then
      local dist  = ai.dist( target )
      local range = ai.getweaprange( 3 )

      -- Shouldn't switch targets if close
      if dist > range * mem.atk_changetarget then
         ai.pushtask( "attack", enemy )
      end
   end
end

--[[
-- Attacked function.  Only called from "attack" tasks (i.e., under "if si.attack").
--]]
function atk_generic.attacked( attacker )
   if mem.recharge then
      mem.recharge = false
   end

   local target = ai.taskdata()
   if not target:exists() then
      ai.pushtask("attack", attacker)
      return
   end
   local dist  = ai.dist(attacker)
   local range = ai.getweaprange( 0 )

   -- Choose target based on preference
   if target ~= attacker and dist < range * mem.atk_changetarget then
      local wtarget = atk.preferred_enemy_test( target )
      local wattacker = atk.preferred_enemy_test( attacker )
      if wattacker < wtarget then -- minimizing
         ai.pushtask("attack", attacker)
      end
   end
end

--[[
-- Approaches the target
--]]
local function __atk_g_approach( target, _dist )
   local dir = ai.idir(target)
   if dir < math.rad(10) and dir > -math.rad(10) then
      atk.keep_distance()
   else
      dir = ai.iface(target)
   end
   if dir < math.rad(10) then
      ai.accel()
   end
end

--[[
-- Melees the target
--]]
local function __atk_g_melee( target, dist )
   local dir   = ai.aim(target) -- We aim instead of face
   local range = ai.getweaprange( 3 )
   ai.weapset( 3 ) -- Set turret/forward weaponset.

   -- Drifting away we'll want to get closer
   if dir < math.rad(10) and dist > 0.5*range and ai.relvel(target) > -10 then
      ai.accel()
   end

   -- Shoot if should be shooting.
   if dir < math.rad(10) then
      ai.shoot()
   end
   ai.shoot(true)

   -- Also try to shoot missiles
   atk.dogfight_seekers( dist, dir )
end

--[[
-- Generic "brute force" attack.  Doesn't really do anything interesting.
--]]
function atk_generic.atk( target, dokill )
   target = atk.com_think( target, dokill )
   if target == nil then return end

   -- Always launch fighters for now
   ai.weapset( 5 )

   ai.hostile(target) -- Mark as hostile

   -- See if the enemy is still seeable
   if not atk.check_seeable( target ) then return end

   -- Targeting stuff
   ai.settarget(target)

   -- Get stats about enemy
   local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange( 3 )

   -- We first bias towards range
   if dist > range * mem.atk_approach and mem.ranged_ammo > mem.atk_minammo then
      atk.ranged( target, dist )

   -- Now we do an approach
   elseif dist > range * mem.atk_aim then
      __atk_g_approach( target, dist )

   -- Close enough to melee
   else
      __atk_g_melee( target, dist )
   end
end

return atk_generic
