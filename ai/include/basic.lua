--[[
-- Basic tasks for a pilot, no need to reinvent the wheel with these.
--
-- Idea is to have it all here and only really work on the "control"
-- functions and such for each AI.
--]]


--[[
-- Attempts to land on a planet.
--]]
function land ()

   -- Only want to land once, prevents guys from never leaving.
   if mem.landed then
      ai.poptask()
      return
   end

   target   = mem.land
   dir      = ai.face( target )
   dist     = ai.dist( target )
   bdist    = ai.minbrakedist()

   -- Need to get closer
   if dir < 10 and dist > bdist then
      ai.accel()

   -- Need to start braking
   elseif dist < bdist then
      ai.poptask()
      ai.pushtask( 0, "landstop" )
   end

end
function landstop ()
   ai.brake()
   if ai.isstopped() then
      ai.stop() -- Will stop the pilot if below err vel
      ai.settimer(0, rnd.int(8000,15000)) -- We wait during a while
      ai.poptask()
      ai.pushtask( 0, "landwait")
   end
end
function landwait ()
   target = mem.land
   dist   = ai.dist( target )

   -- In case for some reason landed far away
   if dist > 50 then
      ai.poptask()
      ai.pushtask( 0, "land" )

   -- Check if time is up
   elseif ai.timeup(0) then
      mem.landed = true -- Mark as landed so they don't spend time forever floating around
      ai.poptask() -- Ready to do whatever we were doing before.
   end
end


--[[
-- Attempts to run away from the target.
--]]
function runaway ()
   target = ai.target()

   -- Target must exist
   if not ai.exists(target) then
      ai.poptask()
      return
   end

   -- Good to set the target for distress calls
   ai.settarget( target )

   dir = ai.face(target, true)
   ai.accel()

   --[[
   -- Todo afterburner handling.
   if ai.hasafterburner() then
      ai.afterburn(true)
   end
   ]]--

   -- See if we have some turret to use
   secondary, special = ai.secondary("melee")
   if special == "Turret" then
      if dist < ai.getweaprange(true) then
         ai.shoot(true)
      end
   end

   if ai.hasturrets() then
      dist = ai.dist( target )
      if dist < ai.getweaprange() then
         ai.shoot(false, 1)
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


--[[
-- Boards the target
--]]
function board ()
   target = ai.target()

   -- Make sure pilot exists
   if not ai.exists(target) then
      ai.poptask()
      return
   end

   -- Must be able to board
   if not ai.canboard(target) then
      ai.poptask()
      return
   end

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



--[[
-- Boards the target
--]]
function refuel ()

   -- Get the target
   target = ai.target()

   -- make sure pilot exists
   if not ai.exists(target) then
      ai.poptask()
      return
   end

   -- See if finished refueling
   if ai.donerefuel(target) then
      ai.poptask()
      return
   end

   -- Get ready to board
   ai.settarget(target)
   dir   = ai.face(target)
   dist  = ai.dist(target)
   bdist = ai.minbrakedist(target)

   -- See if must brake or approach
   if dist < bdist then
      ai.pushtask( 0, "refuelstop", target )
   elseif dir < 10 then
      ai.accel()
   end
end

--[[
-- Attempts to brake on the target.
--]]
function refuelstop ()
   target = ai.target()

   -- make sure pilot exists
   if not ai.exists(target) then
      ai.poptask()
      return
   end

   -- Set the target
   ai.settarget(target)

   -- See if finished refueling
   if ai.donerefuel(target) then
      ai.comm(target, "Finished fuel transfer.")
      ai.poptask()
      return
   end

   -- Try to board
   if ai.refuel(target) then
      return
   end

   -- Just brake
   ai.brake()

   -- If stopped try again
   if ai.isstopped() then
      ai.poptask()
   end
end

