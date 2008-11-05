--[[
--    Generic attack functions
--]]

atk_changetarget = 1.8
atk_approach     = 1.4
atk_aim          = 1.0


--[[
-- Mainly manages targetting nearest enemy.
--]]
function atk_g_think ()
   enemy = ai.getenemy()
   target = ai.target()

   -- Get new target if it's closer
   if enemy ~= target and enemy ~= nil then
      dist = ai.dist( ai.pos(target) )
      range = ai.getweaprange()

      -- Shouldn't switch targets if close
      if dist > 400 and dist > range * atk_changetarget then
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

   -- Check if is bribed by target
   if ai.isbribed(target) then
      ai.poptask()
      return
   end

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
   if dist > range * atk_approach then
      atk_g_ranged( target, dist )

   elseif dist > range * atk_aim then
      if ai.relvel(target) > -10 then
         atk_g_ranged( target, dist )
      else
         atk_g_aim( target, dist )
      end

   -- Close enough to melee
   else
      atk_g_melee( target, dist )
   end
end


--[[
-- Enters ranged combat with the target
--]]
function atk_g_ranged( target, dist )
   dir = ai.face(target) -- Normal face the target
   secondary, special, ammo = ai.secondary("ranged")

   -- Always use fighter bay
   if secondary == "" then

   -- Shoot missiles if in range
   elseif secondary == "Launcher" and
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
end


--[[
-- Aims at the target
--]]
function atk_g_aim( target, dist )
   dir = ai.aim(target)
end


--[[
-- Melees the target
--]]
function atk_g_melee( target, dist )
   secondary, special = ai.secondary("melee")
   dir = ai.aim(target) -- We aim instead of face
   range = ai.getweaprange()

   -- Fire non-smart secondary weapons
   if (secondary == "Launcher" and special ~= "Smart") or
         secondary == "Beam Weapon" then
      if dir < 10 or special == "Turret" then -- Need good acuracy
         ai.shoot(2)
      end
   end

   -- Drifting away we'll want to get closer
   if dir < 10  and dist > 0.5*range and ai.relvel(target) > -10 then
      ai.accel()
   end

   -- Shoot if should be shooting.
   if (dir < 10 and dist < range) or ai.hasturrets() then
      ai.shoot()
   end
end
