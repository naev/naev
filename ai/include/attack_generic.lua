--[[
--    Generic attack functions
--]]

atk_changetarget  = 1.8 -- Distance at which target changes
atk_approach      = 1.4 -- Distance that marks approach
atk_aim           = 1.0 -- Distance that marks aim
atk_board         = false -- Whether or not to board the target
atk_kill          = true -- Whether or not to finish off the target


--[[
-- Mainly manages targetting nearest enemy.
--]]
function atk_g_think ()
   enemy = ai.getenemy()
   target = ai.target()

   -- Stop attacking if it doesn't exist
	if not ai.exists(target) then
		ai.poptask()
		return
	end

   -- Get new target if it's closer
   if enemy ~= target and enemy ~= nil then
      dist = ai.dist( target )
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

	-- make sure pilot exists
	if not ai.exists(target) then
		ai.poptask()
		return
	end

   -- Check if is bribed by target
   if ai.isbribed(target) then
      ai.poptask()
      return
   end

   -- Check if we want to board
   if atk_board and ai.canboard(target) then
      board(target)
      return
   end

   -- Check to see if target is disabled
   if not atk_kill and ai.isdisabled(target) then
      ai.poptask()
      return
   end

   -- Targetting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- Get stats about enemy
	dist = ai.dist( target ) -- get distance
   range = ai.getweaprange()

   -- We first bias towards range
   if dist > range * atk_approach then
      atk_g_ranged( target, dist )

   -- Now we do an approach
   elseif dist > range * atk_aim then
      atk_g_approach( target, dist )

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
         ai.shoot(true)

      -- Non-smart miss more
      elseif dir < 10 then
         ai.shoot(true)
      end
   end

   -- Approach for melee
   if dir < 10 then
      ai.accel()
   end
end


--[[
-- Approaches the target
--]]
function atk_g_approach( target, dist )
   dir = ai.aim(target)
   if dir < 10 then
      ai.accel()
   end
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
         ai.shoot(true)
      end
   end

   -- Drifting away we'll want to get closer
   if dir < 10  and dist > 0.5*range and ai.relvel(target) > -10 then
      ai.accel()
   end

   -- Shoot if should be shooting.
   if dist < range then
      if dir < 10 then
         ai.shoot(false)
      elseif ai.hasturrets() then
         ai.shoot(false, 1)
      end
   end
end


--[[
-- Boards the target
--]]
function board( target )

   -- Get ready to board
   ai.settarget(target)
   dir   = ai.face(target)
   dist  = ai.dist(target)
   bdist = ai.minbrakedist(target)

   -- See if must brake or approach
   if dist < bdist then
      ai.pushtask( 0, "boardstop", target )
   elseif dir < 10 then
      ai.accel()
   end
end


--[[
-- Attempts to brake on the target.
--]]
function boardstop ()
   target = ai.target()

	-- make sure pilot exists
	if not ai.exists(target) then
		ai.poptask()
		return
	end

   ai.settarget(target)
   vel = ai.relvel(target)

   if vel < 10 then
      -- Try to board
      if ai.board(target) then
         ai.poptask()
         return
      end
   end

   -- Just brake
   ai.brake()

   -- If stopped try again
   if ai.isstopped() then
      ai.poptask()
   end
end
