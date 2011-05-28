--[[ Detail melee attack profiles follow here
--These profiles are intended for use by specific ship classes
--]]

--[[
-- Execute a sequence of close-in flyby attacks
-- Uses a combination of facing and distance to determine what action to take
-- This version is slightly less aggressive and cruises by the target
--]]
function atk_g_flyby( target, dist )
   local range = ai.getweaprange(3)
   local dir = 0
   ai.weapset( 3 ) -- Forward/turrets

   -- Far away, must approach
   if dist > (3 * range) then
      dir = ai.idir(target)
      if dir < 10 and dir > -10 then
         keep_distance()     
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
-- Simplest of all attacks: maintain an intercept course to the target, and shoot when within range
--
--This is designed for capital ships with turrets and guided munitions
--As there is no aiming involved this is a turret/capital ship only attack method
--]]
function atk_g_capital( target, dist )
   local range = ai.getweaprange(3)
   local dir = 0
   ai.weapset( 3 ) -- Forward/turrets

   --capital ships tend to require heavier energy reserves and burst output for maximum effectiveness
   if ai.pcurenergy() <= 1 then
      mem.recharge = true
   elseif ai.pcurenergy() > 15 then
      mem.recharge = false
   end

   --if we're far from the target, then turn and approach 
   if dist > range then
      dir = ai.idir(target)
      if dir < 10 and dir > -10 then
         keep_distance()     
         ai.accel()
      else  
         dir = ai.iface(target)
      end

   --at moderate range from the target, prepare to intercept and engage with turrets
   elseif dist > 0.6* range then
      --drifting away from target, so emphasize intercept 
      --course facing and accelerate to close
      dir = ai.iface(target)
      if dir < 10 and dir > -10 and ai.relvel(target) > -10 then 
         ai.accel()
      end
      if mem.recharge == false then
         ai.shoot(true)
      end

   elseif dist > 0.3*range then
      --capital ship turning is slow
      --emphasize facing for being able to close quickly
      dir = ai.iface(target)
      -- Shoot if should be shooting.
      if mem.recharge == false then
         ai.shoot(true)
      end

   --within close range; aim and blast away with everything
   else

      dir = ai.aim(target)
      if mem.recharge == false then        
         -- Shoot if should be shooting.
         if dir < 10 then
            ai.shoot()
         end
         ai.shoot(true)
      end

   end
end


--[[
-- Attack Profile for a maneuverable ship engaging a maneuverable target
--
--This is designed for fighters engaging other fighters
--
--]]
function atk_g_space_sup( target, dist )

   -- ai.comm(1, "space superiority")

   local range = ai.getweaprange(3)
   local dir   = 0
   ai.weapset( 3 ) -- Forward/turrets

   --if we're far away from the target, then turn and approach 
   if dist > (range) then
      dir = ai.idir(target)
      if dir < 10 and dir > -10 then
         keep_distance()     
         ai.accel()
      else  
         dir = ai.iface(target)
      end

   elseif dist > 0.8* range then
      --drifting away from target, so emphasize intercept 
      --course facing and accelerate to close
      dir = ai.iface(target)
      if dir < 10 and dir > -10 and ai.relvel(target) > -10 then
         ai.accel()
      end
      if ai.hasturrets() then
         ai.shoot(false, 1)
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

