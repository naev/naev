--[[This file contains the attack profiles by ship type.
--commonly used range and condition-based attack patterns are found in another file
--Think functions for determining who to attack are found in another file
--]]

local atk = require "ai.core.attack.util"

local __atk_d_flyby, __atk_d_space_sup, __atk_drone_ranged -- Forward-declared functions

local atk_drone = {}

--[[
-- Mainly targets small drones.
--]]
function atk_drone.think( target, _si )
   -- Low chance to not switch targets
   if rnd.rnd() < 0.2 then
      return
   end

   -- Don't switch targets if close to current one
   local dist  = ai.dist( target )
   local range = ai.getweaprange(3, 0)
   if dist < range * mem.atk_changetarget then
      return
   end

   -- Prioritize preferred target
   local enemy = atk.preferred_enemy( atk.prefer_capship )
   if enemy ~= target and enemy ~= nil then
      ai.pushtask("attack", enemy )
      return
   end
end


--[[
-- Enters ranged combat with the target - modified version for drones
--]]
function __atk_drone_ranged( target, dist )
   --local dir = ai.face(target) -- Normal face the target
  local dir = ai.aim(target) -- Aim for the target
   -- TODO: should modify this line

   -- Check if in range
   if dist < ai.getweaprange( 4 ) then
      if dir < math.rad(30) then
         ai.weapset( 4 ) -- Weaponset 4 contains weaponset 9
      else
         ai.weapset( 9 )
      end
   else
      -- First test if we should zz
      if atk.decide_zz( target, dist ) then
         ai.pushsubtask("_attack_zigzag", target)
      end
   end

   -- Always launch fighters
   ai.weapset( 5 )

   -- Approach for melee
   if dir < math.rad(10) then
      ai.accel()
   end
end

--[[
-- Main control function for drone behavior.
--]]
function atk_drone.atk( target, dokill )
   target = atk.com_think( target, dokill )
   if target == nil then return end

   -- Targeting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- See if the enemy is still seeable
   if not atk.check_seeable( target ) then return end

   -- Get stats about enemy
   local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange(3, 0)  -- get my weapon range (?)

   -- We first bias towards range
   if dist > range * mem.atk_approach then
      __atk_drone_ranged( target, dist ) -- Use generic ranged function

   -- Otherwise melee
   else
      if target:mass() < 200 then
         __atk_d_space_sup( target, dist )
      else
         __atk_d_flyby( target, dist )
      end
   end
end


--[[
-- Approaches the target evasively, never heading in a straight line
-- This will tend to approach a target along a loose spiral, good for evading capship guns
-- HISTORICAL NOTE: was removed in commit 5b3d7e12f "suppress useless stuff", which may or may not have been fair.
--]]
local function atk_spiral_approach( target, _dist )
   local dir  = ai.idir(target)
   local adir = math.abs(dir)

   --these two detect in-cone approach vectors
   if adir > math.rad(10) and adir < math.rad(30) then
      ai.accel()
   end

   --facing away from the target, turn to face
   if adir > math.rad(30) then
      ai.iface(target)
   end

   --aiming right at the target; turn away
   if dir > 0 and dir < math.rad(10) then
      ai.turn(1)
   elseif dir < 0 and dir > -math.rad(10) then
      ai.turn(-1)
   end
end -- end spiral approach


--[[
-- Execute a sequence of close-in flyby attacks
-- Uses a combination of facing and distance to determine what action to take
-- This version is slightly less aggressive and cruises by the target
--]]
function __atk_d_flyby( target, dist )
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
         --atk.keep_distance()
         atk_spiral_approach(target, dist)  -- mod
         ai.accel()
      else
         ai.iface(target)
      end

   -- Midrange
   elseif dist > (0.75 * range) then
      --dir = ai.idir(target)
      dir = ai.aim(target)  -- drones need to aim more to avoid circling
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

   end
end


--[[
-- Attack Profile for a maneuverable ship engaging a maneuverable target
--
--This is designed for drones engaging other drones
--]]
function __atk_d_space_sup( target, dist )
   local range = ai.getweaprange(3)
   local dir
   ai.weapset( 3 ) -- Forward/turrets

   -- First test if we should zz
   if atk.decide_zz( target, dist ) then
      ai.pushsubtask("_attack_zigzag", target)
   end

   --if we're far away from the target, then turn and approach
   if dist > (1.1*range) then
      dir = ai.idir(target)
      if dir < math.rad(10) and dir > -math.rad(10) then
         atk.keep_distance()
         ai.accel()
      else
         ai.iface(target)
         ai.accel()
      end

   elseif dist > 0.8* range then
      --drifting away from target, so emphasize intercept
      --course facing and accelerate to close
      --dir = ai.iface(target)
      dir = ai.aim(target)
      if dir < math.rad(15) and dir > -math.rad(15) then
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

   --within really close range (?); aim and blast away with everything
   else
      dir = ai.aim(target)
      -- Shoot if should be shooting.
      if dir < math.rad(15) then  -- mod: was 10
         ai.shoot()
      end
      ai.shoot(true)
   end
end

return atk_drone
