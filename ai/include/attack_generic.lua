--[[
--    Generic attack functions
--]]


--[[
-- Mainly manages targetting nearest enemy.
--]]
function atk_g_think ()
   enemy = ai.getenemy()
   target = ai.target()

   -- Get new target if it's closer
   if enemy ~= target then
      dist = ai.dist( ai.pos(target) )
      range = ai.getweaprange()

      -- Shouldn't switch targets if close
      if dist > range * 1.6 then
         ai.poptask()
         ai.pushtask( 0, "attack", enemy )
      end
   end
end


--[[
-- Generic "brute force" attack.  Doesn't really do anything interesting.
--]]
function atk_g ()
	target = ai.target()
   ai.hostile(target) -- Mark as hostile

	-- make sure pilot exists
	if not ai.exists(target) then
		ai.poptask()
		return
	end
   ai.settarget(target)

   -- Get stats about enemy
	dist = ai.dist( ai.pos(target) ) -- get distance
   range = ai.getweaprange()

   -- We first bias towards range
   if dist > range*1.3 then
      dir = ai.face(target) -- Normal face the target

      secondary, special, ammo = ai.secondary("Launcher")

      -- Shoot missiles if in range
      if secondary == "Launcher" and
            dist < ai.getweaprange(1) then

         -- More lenient with aiming
         if special == "Smart" and dir < 30 then
            ai.shoot(2)

         -- Non-smart miss more
         elseif dir < 10 then
            ai.shoot(2)
         end
      end

      -- Approach for melee
      if dir < 10 then
         ai.accel()
      end

   -- Close enough to melee
   else

      secondary, special = ai.secondary("Beam Weapon")
      dir = ai.aim(target) -- We aim instead of face

      -- Fire non-smart secondary weapons
      if (secondary == "Launcher" and special ~= "Smart") or
            secondary == "Beam Weapon" then
         if dir < 10 or special == "Turret" then -- Need good acuracy
            ai.shoot(2)
         end
      end

      if (dir < 10 and dist < range)or ai.hasturrets() then
         ai.shoot()
      end
   end
end

