--[[This file contains the attack profiles by ship type.
--commonly used range and condition-based attack patterns are found in another file
--Think functions for determining who to attack are found in another file
--]]

local atk = require "ai.core.attack.util"


--[[
-- Mainly targets small fighters.
--]]
local function atk_fighter_think( target, _si )
   local enemy    = ai.getenemy_size(0, 200)
   local nearest_enemy = ai.getenemy()
   local dist     = ai.dist(target)

   local range = ai.getweaprange(3, 0)
   -- Get new target if it's closer
   --prioritize targets within the size limit
   if enemy ~= target and enemy ~= nil then
      -- Shouldn't switch targets if close
      if dist > range * mem.atk_changetarget then
         ai.pushtask("attack", enemy )
      end

   elseif nearest_enemy ~= target and nearest_enemy ~= nil then
      -- Shouldn't switch targets if close
      if dist > range * mem.atk_changetarget then
         ai.pushtask("attack", nearest_enemy )
      end
   end
end


--[[
-- Main control function for fighter behavior.
--]]
local function atk_fighter( target, dokill )
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
      __atk_g_ranged( target, dist ) -- Use generic ranged function

   -- Otherwise melee
   else
      if target:stats().mass < 200 then
         __atk_f_space_sup( target, dist )
      else
         __atk_f_flyby( target, dist )
      end
   end
end


--[[
-- Execute a sequence of close-in flyby attacks
-- Uses a combination of facing and distance to determine what action to take
-- This version is slightly less aggressive and cruises by the target
--]]
function __atk_f_flyby( target, dist )
   local range = ai.getweaprange(3)
   local dir
   ai.weapset( 3 ) -- Forward/turrets

   -- First test if we should zz
   if atk.decide_zz( target, dist ) then
      ai.pushsubtask("_attack_zigzag", target)
   end

   -- Far away, must approach
   if dist > (3 * range) then
      dir = ai.idir(target)
      if dir < math.rad(10) and dir > -math.rad(10) then
         atk.keep_distance()
         ai.accel()
      else
         ai.iface(target)
      end

   -- Midrange
   elseif dist > (0.75 * range) then
      dir = ai.idir(target)
      --test if we're facing the target. If we are, keep approaching
      if dir <= math.rad(30) and dir > -math.rad(30) then
         ai.iface(target)
         if dir < math.rad(10) and dir > -math.rad(10) then
            ai.accel()
         end
      elseif dir > math.rad(30) and dir < math.pi then
         ai.turn(1)
         ai.accel()
      else
         ai.turn(-1)
         ai.accel()
      end

   --otherwise we're close to the target and should attack until we start to zip away
   else
      dir = ai.aim(target)
      --not accelerating here is the only difference between the aggression levels. This can probably be an aggression AI parameter
      if mem.aggressive == true then
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
end


--[[
-- Attack Profile for a maneuverable ship engaging a maneuverable target
--
--This is designed for fighters engaging other fighters
--]]
function __atk_f_space_sup( target, dist )
   local range = ai.getweaprange(3)
   local dir
   ai.weapset( 3 ) -- Forward/turrets

   -- First test if we should zz
   if atk.decide_zz( target, dist ) then
      ai.pushsubtask("_attack_zigzag", target)
   end

   --if we're far away from the target, then turn and approach
   if dist > (range) then
      dir = ai.idir(target)
      if dir < math.rad(10) and dir > -math.rad(10) then
         atk.keep_distance()
         ai.accel()
      else
         ai.iface(target)
      end

   elseif dist > 0.8* range then
      --drifting away from target, so emphasize intercept
      --course facing and accelerate to close
      dir = ai.iface(target)
      if dir < math.rad(10) and dir > -math.rad(10) then
         ai.accel()
      end

   --within close range; aim and blast away with everything
   elseif dist > 0.4*range then
      dir = ai.aim(target)
      local dir2 = ai.idir(target)

      --accelerate and try to close
      --but only accel if it will be productive
      if dir2 < math.rad(15) and dir2 > -math.rad(15) and ai.relvel(target) > -math.rad(10) then
         ai.accel()
      end

      -- Shoot if should be shooting.
      if dir < math.rad(10) then
         ai.shoot()
      end
      ai.shoot(true)

      -- Also try to shoot missiles
      atk.dogfight_seekers( dist, dir )

   --within close range; aim and blast away with everything
   else
      dir = ai.aim(target)
      -- Shoot if should be shooting.
      if dir < math.rad(10) then
         ai.shoot()
      end
      ai.shoot(true)
   end
end


-- Initializes the fighter
function atk_fighter_init ()
   mem.atk_think  = atk_fighter_think
   mem.atk        = atk_fighter
end
