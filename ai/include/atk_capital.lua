function atk_capital_init ()
   mem.atk_think  = atk_heuristic_big_game_think
   mem.atk        = atk_capital
end


--[[
-- Main control function for capital ship behavior.
--]]
function atk_capital ()
   local target = _atk_com_think()
   if target == nil then return end

   -- Targeting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- Get stats about enemy
   local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange(3)

   -- We first bias towards range
   if dist > range * mem.atk_approach then
      _atk_g_ranged( target, dist )
   -- Close enough to melee
   else   
     _atk_g_capital(target, dist)
   end
end


--[[
-- Simplest of all attacks: maintain an intercept course to the target, and shoot when within range
--
--This is designed for capital ships with turrets and guided munitions
--As there is no aiming involved this is a turret/capital ship only attack method
--]]
function _atk_g_capital( target, dist )
   local range = ai.getweaprange(3)
   local dir = 0
   ai.weapset( 3 ) -- Forward/turrets

   --capital ships tend to require heavier energy reserves and burst output for maximum effectiveness
   if ai.pcurenergy() <= 1 then
      mem.recharge = true
   elseif ai.pcurenergy() > 15 then
      mem.recharge = false
   end

   -- Don't go on the offensive when in the middle of recharging or cooling.
   if mem.recharge or mem.cooldown then
      return
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
      ai.shoot(true)

   elseif dist > 0.3*range then
      --capital ship turning is slow
      --emphasize facing for being able to close quickly
      dir = ai.iface(target)
      -- Shoot if should be shooting.
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


