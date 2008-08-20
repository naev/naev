--[[
-- Basic tasks for a pilot, no need to reinvent the wheel with these.
--
-- Idea is to have it all here and only really work on the "control"
-- functions and such for each AI.
--]]


--[[
-- Attacks the current target, task pops when target is dead.
--]]
function attack_default ()
	local target = ai.targetid()

	-- make sure pilot exists
	if not ai.exists(target) then
		ai.poptask()
		return
	end
   ai.settarget(target)

   -- Get stats about enemy
	local dist = ai.dist( ai.pos(target) ) -- get distance
   local range = ai.getweaprange()

   -- We first bias towards range
   if dist > range then
      local dir = ai.face(target) -- Normal face the target

      local secondary, special = ai.secondary("Launcher")

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

      local secondary, special = ai.secondary("Beam Weapon")
      local dir = ai.aim(target) -- We aim instead of face

      -- Fire non-smart secondary weapons
      if (secondary == "Launcher" and special ~= "Smart") or
            secondary == "Beam Weapon" then
         if dir < 10 or special == "Turret" then -- Need good acuracy
            ai.shoot(2)
         end
      end

      if dir < 10 or ai.hasturrets() then
         ai.shoot()
      end
   end
end


--[[
-- Set attack function to be default.  If you want to override use:
-- attack = attack_<type>
-- Right after including this file.
--]]
attack = attack_default


--[[
-- Attempts to land on a planet.
--]]
function land ()
   local target = ai.target()
   local dir = ai.face(target)
   local dist = ai.dist( target )
   local bdist = ai.minbrakedist()

   -- Need to get closer
   if dir < 10 and dist > bdist then
      ai.accel()

   -- Need to start braking
   elseif dist < bdist then
      ai.poptask()
      ai.pushtask( 0, "landstop", target )

   end

end
function landstop ()
   ai.brake()
   if ai.isstopped() then
      local target = ai.target()

      ai.stop() -- Will stop the pilot if below err vel
      ai.settimer(0, rnd.int(8000,15000)) -- We wait during a while
      ai.poptask()
      ai.pushtask( 0, "landwait", target )
   end
end
function landwait ()
   local target = ai.target()
   local dist = ai.dist( target )

   -- In case for some reason landed far away
   if dist > 50 then
      ai.poptask()
      ai.pushtask( 0, "land", target )

   -- Check if time is up
   elseif ai.timeup(0) then
      ai.poptask() -- Ready to do whatever we were doing before.
   end
end


--[[
-- Attempts to run away from the target.
--]]
function runaway ()
   local target = ai.targetid()
   
   if not ai.exists(target) then
      ai.poptask()
      return
   end
   
   local dir = ai.face( target, 1 )
   ai.accel()

   --[[
   -- Todo afterburner handling.
   if ai.hasafterburner() then
      ai.afterburn(true)
   end
   ]]--

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
-- local task = ai.taskname()
-- if task == "hyperspace" then
--    ai.hyperspace() -- Try to hyperspace
-- end
--]]
function hyperspace ()
   local dir = ai.face(-1) -- face away from (0,0)
   if (dir < 10) then
      ai.accel()
   end
end



