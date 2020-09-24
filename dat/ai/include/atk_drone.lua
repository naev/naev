--[[This file contains the attack profiles by ship type.
--commonly used range and condition-based attack patterns are found in another file
--Think functions for determining who to attack are found in another file
--]]

-- Initializes the drone
function atk_drone_init ()
   mem.atk_think  = atk_drone_think
   mem.atk        = atk_drone
end


--[[
-- Mainly targets small drones.
--]]
function atk_drone_think ()
   local target = ai.target()

   -- Stop attacking if it doesn't exist
   if not target:exists() then
      ai.poptask()
      return
   end

   local enemy    = ai.getenemy_size(0, 200)  -- find a small ship to attack
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
-- Enters ranged combat with the target - modified version for drones
--]]
function _atk_drone_ranged( target, dist )
   --local dir = ai.face(target) -- Normal face the target
  local dir = ai.aim(target) -- Aim for the target
   -- TODO: should modify this line

   -- Check if in range
   if dist < ai.getweaprange( 4 ) and dir < 30 then
      ai.weapset( 4 )
   else
      -- First test if we should zz
      if _atk_decide_zz() then
         ai.pushsubtask("_atk_zigzag")
      end
   end

   -- Always launch fighters
   ai.weapset( 5 )

   -- Approach for melee
   if dir < 10 then
      ai.accel()
   end
end

--[[
-- Main control function for drone behavior.
--]]
function atk_drone ()
   local target = _atk_com_think()
   if target == nil then return end

   -- Targeting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- Get stats about enemy
   local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange(3, 0)  -- get my weapon range (?)

   -- We first bias towards range
   if dist > range * mem.atk_approach then
      _atk_drone_ranged( target, dist ) -- Use generic ranged function

   -- Otherwise melee
   else
      if target:stats().mass < 200 then
         _atk_d_space_sup( target, dist )
      else
         _atk_d_flyby( target, dist )
      end
   end
end


--[[
-- Execute a sequence of close-in flyby attacks
-- Uses a combination of facing and distance to determine what action to take
-- This version is slightly less aggressive and cruises by the target
--]]
function _atk_d_flyby( target, dist )
   local range = ai.getweaprange(3)
   local dir = 0
   ai.weapset( 3 ) -- Forward/turrets

   -- First test if we should zz
   if _atk_decide_zz() then
      ai.pushsubtask("_atk_zigzag")
   end

   -- Far away, must approach
   if dist > (3 * range) then
      dir = ai.idir(target)
      if dir < 10 and dir > -10 then
         --_atk_keep_distance()
         atk_spiral_approach(target, dist)  -- mod
         ai.accel()
      else
         dir = ai.iface(target)
      end

   -- Midrange
   elseif dist > (0.75 * range) then

      --dir = ai.idir(target)
      dir = ai.aim(target)  -- drones need to aim more to avoid circling
      --test if we're facing the target. If we are, keep approaching
      if dir <= 30 and dir > -30 then
         ai.iface(target)
         if dir < 10 and dir > -10 then
            ai.accel()
         end
      elseif dir > 30 and dir < 180 then
         ai.turn(1)
         ai.accel()
      else
         ai.turn(-1)
         ai.accel()
      end

   --otherwise we're close to the target and should attack until we start to zip away
   else

      dir = ai.aim(target)
      --not accellerating here is the only difference between the aggression levels. This can probably be an aggression AI parameter
      if mem.aggressive == true then
         ai.accel()
      end

      -- Shoot if should be shooting.
      if dir < 10 then
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
function _atk_d_space_sup( target, dist )
   local range = ai.getweaprange(3)
   local dir   = 0
   ai.weapset( 3 ) -- Forward/turrets

   -- First test if we should zz
   if _atk_decide_zz() then
      ai.pushsubtask("_atk_zigzag")
   end

   --if we're far away from the target, then turn and approach
   if dist > (1.1*range) then
      dir = ai.idir(target)
      if dir < 10 and dir > -10 then
         _atk_keep_distance()
         ai.accel()
      else
         dir = ai.iface(target)
         ai.accel()
      end

   elseif dist > 0.8* range then
      --drifting away from target, so emphasize intercept
      --course facing and accelerate to close
      --dir = ai.iface(target)
      dir = ai.aim(target)
      if dir < 15 and dir > -15 then
         ai.accel()
      end

   --within close range; aim and blast away with everything
   elseif dist > 0.4*range then
      dir = ai.aim(target)
      local dir2 = ai.idir(target)

      --accelerate and try to close
      --but only accel if it will be productive
      if dir2 < 15 and dir2 > -15 and ai.relvel(target) > -10 then
         ai.accel()
      end

      -- Shoot if should be shooting.
      if dir < 10 then
         ai.shoot()
      end
      ai.shoot(true)

   --within really close range (?); aim and blast away with everything
   else
      dir = ai.aim(target)
      -- Shoot if should be shooting.
      if dir < 15 then  -- mod: was 10
         ai.shoot()
      end
      ai.shoot(true)
   end
end


