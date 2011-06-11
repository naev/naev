--[[ Detail melee attack profiles follow here
--These profiles are intended for use by specific ship classes
--]]



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
         _atk_keep_distance()     
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


