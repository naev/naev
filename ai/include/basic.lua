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

   -- Get stats about enemy
	dir = ai.face( target ) -- face target
	dist = ai.dist( ai.pos(target) ) -- get distance
	second = ai.secondary() -- get best secondary weapon

   -- Shoot missiles if in range
	if ai.secondary() == "Launcher" and dist < ai.getweaprange(1) then
		ai.settarget(target)
		ai.shoot(2)
	end

   -- Attack if in range
   range = ai.getweaprange()
	if dir < 10 and dist > range then
		ai.accel()
	elseif (dir < 10 or ai.hasturrets()) and dist < range then
		ai.shoot()
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
      if dist < 300 then
         ai.settarget(target)
         ai.shoot()
      end
   end
end



