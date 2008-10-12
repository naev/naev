--[[
--    Attack functions for bombers
--]]


--[[
-- Bombers don't really think, they lock on until target is dead.
--]]
function atk_b_think ()
   -- No thinking atm
end


--[[
-- Attacks the current target, task pops when target is dead.
--
-- Specialized for bomber type craft.  AI will try to shoot missiles and such
--  until out and then will melee.
--]]
function atk_b ()
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

   -- Get bombing tool
   secondary, special = ai.secondary("Launcher")
   if secondary ~= "Launcher" or special == "Dumb" then -- No launcher, must melee
      range = ai.getweaprange()

      -- Must approach
      if dist > range then
         dir = ai.face(target)
         if dir < 10 then
            ai.accel()
         end

      -- Time to shoot
      else
         dir = ai.aim(target) -- We aim instead of face
         
         -- Fire secondary
         if dir < 10 or special == "Turret" then
            ai.shoot(1)
         end

         -- Fire primary
         if dir < 10 or ai.hasturrets() then
            ai.shoot()
         end
      end

      return -- No need to do ranged attack calculations
   end

   -- Get ranges relative to bombing weapon of choice
   bombrange = ai.getweaprange(1)
   backoff = bombrange / 4

   -- Must get closer to be able to bomb
   if dist > bombrange then
      dir = ai.face(target)
      if dir < 10 then
         ai.accel()
      end

   -- In bombing range
   elseif dist > backoff then
      dir = ai.face(target)

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

      -- We don't approach, we try to stay away from melee

   -- Time to break attack and get back to bomb
   else
      range = ai.getweaprange()

      -- Flee
      ai.face(target, true)
      ai.accel()

      -- Fire turret if being chased
      if dist < range and ai.hasturrets() then
         ai.shoot()
      end
   end
end
