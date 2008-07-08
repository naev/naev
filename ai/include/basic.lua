--[[
-- Basic tasks for a pilot, no need to reinvent the wheel with these.
--
-- Idea is to have it all here and only really work on the "control"
-- functions and such for each AI.
--]]


--[[
-- Attacks the current target, task pops when target is dead.
--]]
function attack ()
	target = ai.targetid()

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
   if dist > range then
      dir = ai.face(target) -- Normal face the target

      secondary, special = ai.secondary("Launcher")

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

      if dir < 10 then
         ai.accel()
      end

   -- Close enough to melee
   else

      secondary, special = ai.secondary("Weapon")
      dir = ai.aim(target) -- We aim instead of face

      -- Fire non-smart secondary weapons
      if (secondary == "Launcher" and special ~= "Smart") or
            secondary == "Weapon" then
         if dir < 10 then -- Need good acuracy
            ai.shoot(2)
         end
      end

      if dir < 10 or ai.hasturrets() then
         ai.shoot()
      end
   end
end


--[[
-- Attempts to run away from the target.
--]]
function runaway ()
   target = ai.targetid()
   
   if not ai.exists(target) then
      ai.poptask()
      return
   end
   
   dir = ai.face( target, 1 )
   ai.accel()
   if ai.hasturrets() then
      dist = ai.dist( ai.pos(target) )
      if dist < ai.getweaprange() then
         ai.settarget(target)
         ai.shoot()
      end
   end
end


--[[
-- Starts heading away to try to hyperspace.
--
-- Will need teh following in control() to work:
--
-- task = ai.taskname()
-- if task == "hyperspace" then
--    ai.hyperspace() -- Try to hyperspace
-- end
--]]
function hyperspace ()
   dir = ai.face(-1) -- face away from (0,0)
   if (dir < 10) then
      ai.accel()
   end
end



