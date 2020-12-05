--[[This file contains the attack profiles by ship type.
--commonly used range and condition-based attack patterns are found in another file
--Think functions for determining who to attack are found in another file
--]]

-- Initializes the fighter
function atk_fighter_init ()
   mem.atk_think  = atk_fighter_think
   mem.atk        = atk_fighter
end


--[[
-- Mainly targets small fighters.
--]]
function atk_fighter_think ()
   local target = ai.target()

   -- Stop attacking if it doesn't exist
   if not target:exists() then
      ai.poptask()
      return
   end

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
function atk_fighter ()
   local target = _atk_com_think()
   if target == nil then return end

   -- Targeting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- See if the enemy is still seeable
   if not _atk_check_seeable() then return end

   -- Get stats about enemy
   local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange(3, 0)

   -- We first bias towards range
   if dist > range * mem.atk_approach then
      _atk_g_ranged( target, dist ) -- Use generic ranged function

   -- Otherwise melee
   else
      if target:stats().mass < 200 then
         _atk_f_space_sup( target, dist )
      else
         _atk_f_flyby( target, dist )
      end
   end
end


--[[
-- Execute a sequence of close-in flyby attacks
-- Uses a combination of facing and distance to determine what action to take
-- This version is slightly less aggressive and cruises by the target
--]]
function _atk_f_flyby( target, dist )
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
         _atk_keep_distance()     
         ai.accel()
      else  
         dir = ai.iface(target)
      end

   -- Midrange
   elseif dist > (0.75 * range) then

      dir = ai.idir(target)
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
      --not accelerating here is the only difference between the aggression levels. This can probably be an aggression AI parameter
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
--This is designed for fighters engaging other fighters
--]]
function _atk_f_space_sup( target, dist )
   local range = ai.getweaprange(3)
   local dir   = 0
   ai.weapset( 3 ) -- Forward/turrets

   -- First test if we should zz
   if _atk_decide_zz() then
      ai.pushsubtask("_atk_zigzag")
   end

   --if we're far away from the target, then turn and approach 
   if dist > (range) then
      dir = ai.idir(target)
      if dir < 10 and dir > -10 then
         _atk_keep_distance()     
         ai.accel()
      else  
         dir = ai.iface(target)
      end

   elseif dist > 0.8* range then
      --drifting away from target, so emphasize intercept 
      --course facing and accelerate to close
      dir = ai.iface(target)
      if dir < 10 and dir > -10 then
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

   --within close range; aim and blast away with everything
   else
      dir = ai.aim(target)
      -- Shoot if should be shooting.
      if dir < 10 then
         ai.shoot()
      end
      ai.shoot(true)
   end
end


