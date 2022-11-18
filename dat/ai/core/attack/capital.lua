local atk = require "ai.core.attack.util"

local __atk_g_capital -- Forward-declared functions

local atk_capital = {}

function atk_capital.init ()
   mem.atk_pref_func = atk.prefer_capship
end

--[[
-- Main control function for capital ship behavior.
--]]
function atk_capital.atk( target, dokill )
   target = atk.com_think( target, dokill )
   if target == nil then return end

   -- Targeting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- See if the enemy is still seeable
   if not atk.check_seeable( target ) then return end

   -- Get stats about enemy
   local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange(3)

   -- We first bias towards range
   if dist > range * mem.atk_approach and mem.ranged_ammo > mem.atk_minammo then
      atk.ranged( target, dist )
   -- Close enough to melee
   else
     __atk_g_capital(target, dist)
   end
end


--[[
-- Simplest of all attacks: maintain an intercept course to the target, and shoot when within range
--
--This is designed for capital ships with turrets and guided munitions
--As there is no aiming involved this is a turret/capital ship only attack method
--]]
function __atk_g_capital( target, dist )
   local range = ai.getweaprange(3)
   local aimdir, dir
   local shoot = false

   -- Always launch fighters for now
   ai.weapset( 5 )

   -- Also try to shoot missiles
   aimdir = ai.aim(target)
   atk.dogfight_seekers( dist, aimdir )

   -- Set main weapon set
   ai.weapset( mem.weapset )

   --capital ships tend to require heavier energy reserves and burst output for maximum effectiveness
   if ai.pilot():energy() <= 1 then
      mem.recharge = true
   elseif ai.pilot():energy() > 15 then
      mem.recharge = false
   end

   --if we're far from the target, then turn and approach
   if dist > range then
      dir = ai.idir(target)
      if dir < math.rad(10) and dir > -math.rad(10) then
         atk.keep_distance()
         ai.accel()
      else
         ai.iface(target)
      end

   --at moderate range from the target, prepare to intercept and engage with turrets
   elseif dist > 0.6* range then
      --drifting away from target, so emphasize intercept
      --course facing and accelerate to close
      dir    = ai.iface(target)
      if dir < math.rad(10) and dir > -math.rad(10) then
         ai.accel()
      end
      shoot = true

   elseif dist > 0.3*range then
      --capital ship turning is slow
      --emphasize facing for being able to close quickly
      dir    = ai.iface(target)
      -- Only accelerate if the target is getting away.
      if dir < math.rad(10) and dir > -math.rad(10) and ai.relvel(target) > -math.rad(10) then
         ai.accel()
      end
      -- Shoot if should be shooting.
      shoot = true

   --within close range; aim and blast away with everything
   else
      -- At point-blank range, we ignore recharge.
      if aimdir < math.rad(10) then
         ai.shoot()
      end
      ai.shoot(true)
   end

   if shoot then
      if not mem.recharge then
         -- test if, by chance, the target can be hit by cannons
         if aimdir < math.rad(10) then
            ai.shoot()
         end
         ai.shoot(true)
      end
   end
end

atk_capital.atk_think = atk.heuristic_big_game_think

return atk_capital
