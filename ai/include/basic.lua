--[[
-- Basic tasks for a pilot, no need to reinvent the wheel with these.
--
-- Idea is to have it all here and only really work on the "control"
-- functions and such for each AI.
--]]


--[[
-- Brakes the ship
--]]
function brake ()
   ai.brake()
   if ai.isstopped() then
      ai.stop()
      ai.poptask()
      return
   end
end


--[[
-- Goes to a target position without braking
--]]
function __goto_nobrake ()
   local target   = ai.target()
   local dir      = ai.face( target, nil, true )
   local dist     = ai.dist( target )

   -- Need to get closer
   if dir < 10 then
      ai.accel()

   -- Need to start braking
   elseif dist < 50 then
      ai.poptask()
      return
   end
end


--[[
-- Goes to a target position roughly
--]]
function goto ()
   local target   = ai.target()
   local dir      = ai.face( target, nil, true )
   local dist     = ai.dist( target )
   local bdist    = ai.minbrakedist()

   -- Need to get closer
   if dir < 10 and dist > bdist then
      ai.accel()

   -- Need to start braking
   elseif dist < bdist then
      ai.poptask()
      ai.pushtask("brake")
      return
   end
end


--[[
-- Follows it's target.
--]]
function follow ()
   local target = ai.target()

   -- Will just float without a target to escort.
   if not ai.exists(target) then
      return
   end
   
   local dir   = ai.face(target)
   local dist  = ai.dist(target)
   local bdist = ai.minbrakedist()

   -- Close enough.
   if ai.isstopped() and dist < 300 then
      return

   -- Brake
   elseif dist+100 < bdist then
      ai.pushtask("brake")

   -- Must approach
   elseif dir < 10 and dist > 300 then
      ai.accel()

   end
end


--[[
-- Tries to runaway and jump asap.
--]]
function __runaway ()
   runaway()
end


--[[
-- Tries to hyperspace asap.
--]]
function __hyperspace ()
   hyperspace()
end


function __land ()
   land()
end


--[[
-- Attempts to land on a planet.
--]]
function land ()

   -- Only want to land once, prevents guys from never leaving.
   if mem.landed then
      ai.poptask()
      return
   end

   -- Set target if necessary
   local target = ai.target()
   if target ~= nil then
      mem.land = target
   end

   -- Make sure mem.land is valid target
   if mem.land == nil then
      mem.land = ai.landplanet()
   end

   ai.pushsubtask( "__landgo" )
end
function __landgo ()
   local target   = mem.land
   local dir      = ai.face( target )
   local dist     = ai.dist( target )
   local bdist    = ai.minbrakedist()

   -- Need to get closer
   if dir < 10 and dist > bdist then
      ai.accel()

   -- Need to start braking
   elseif dist < bdist then
      ai.pushsubtask( "__landstop" )
   end

end
function __landstop ()
   ai.brake()
   if ai.isstopped() then
      ai.stop() -- Will stop the pilot if below err vel
      if not ai.land() then
         ai.popsubtask()
      end
   end
end


--[[
-- Attempts to run away from the target.
--]]
function runaway ()
   local target = ai.target()

   -- Target must exist
   if not ai.exists(target) then
      ai.poptask()
      return
   end

   -- Good to set the target for distress calls
   ai.settarget( target )

   local dir   = ai.face(target, true)
   ai.accel()

   --[[
   -- Todo afterburner handling.
   if ai.hasafterburner() then
      ai.afterburn(true)
   end
   ]]--

   ai.pushsubtask( "__run_hyp", ai.nearhyptarget() )
end
function __run_turret( dist )
   -- See if we have some turret to use
   local secondary, special = ai.secondary("melee")
   if special == "Turret" then
      if dist < ai.getweaprange(true) then
         ai.shoot(true)
      end
   end

   if ai.hasturrets() then
      if dist < ai.getweaprange() then
         ai.shoot(false, 1)
      end
   end
end
function __run_hyp ()
   -- Shoot the target
   local target   = ai.target()
   if ai.exists(target) then
      ai.settarget( target )
      local tdist    = ai.dist(target)
      __run_turret( tdist )
   end

   -- Go towards jump
   local jump     = ai.subtarget()
   local jdir     = ai.face(jump)
   local bdist    = ai.minbrakedist()
   local jdist    = ai.dist(jump)
   if jdir < 10 and jdist > bdist then
      ai.accel()
   elseif jdist < bdist then
      ai.pushsubtask( "__run_hypbrake" )
   end
end
function __run_hypbrake ()
   -- Shoot the target
   local target   = ai.target()
   if ai.exists(target) then
      ai.settarget( target )
      local tdist    = ai.dist(target)
      __run_turret( tdist )
   end

   -- The braking
   ai.brake()
   if ai.isstopped() then
      ai.stop()
      ai.popsubtask()
      ai.pushsubtask( "__hyp_jump" )
   end
end


--[[
-- Starts heading away to try to hyperspace.
--]]
function hyperspace ()
   local target = ai.target()
   if target == nil then
      target = ai.rndhyptarget()
   end
   ai.pushsubtask( "__hyp_approach", target )
end
function __hyp_approach ()
   local target   = ai.subtarget()
   local dir      = ai.face( target )
   local dist     = ai.dist( target )
   local bdist    = ai.minbrakedist()

   -- Need to get closer
   if dir < 10 and dist > bdist then
      ai.accel()
   -- Need to start braking
   elseif dist < bdist then
      ai.pushsubtask("__hyp_brake")
   end
end
function __hyp_brake ()
   ai.brake()
   if ai.isstopped() then
      ai.stop()
      ai.popsubtask()
      ai.pushsubtask("__hyp_jump")
   end
end
function __hyp_jump ()
   if ai.hyperspace() == nil then
      ai.poptask()
   else
      ai.popsubtask()
   end
end


--[[
-- Boards the target
--]]
function board ()
   local target = ai.target()

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
   local dir   = ai.face(target)
   local dist  = ai.dist(target)
   local bdist = ai.minbrakedist(target)

   -- See if must brake or approach
   if dist < bdist then
      ai.pushsubtask( "__boardstop", target )
   elseif dir < 10 then
      ai.accel()
   end
end


--[[
-- Attempts to brake on the target.
--]]
function __boardstop ()
   target = ai.target()

   -- make sure pilot exists
   if not ai.exists(target) then
      ai.poptask()
      return
   end

   -- Make sure can board
   if not ai.canboard(target) then
      ai.poptask()
      return
   end

   -- Set target
   ai.settarget(target)
   local vel = ai.relvel(target)

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
      ai.popsubtask()
   end
end



--[[
-- Boards the target
--]]
function refuel ()

   -- Get the target
   local target = ai.target()

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
   local dir   = ai.face(target)
   local dist  = ai.dist(target)
   local bdist = ai.minbrakedist(target)

   -- See if must brake or approach
   if dist < bdist then
      ai.pushsubtask( "__refuelstop", target )
   elseif dir < 10 then
      ai.accel()
   end
end

--[[
-- Attempts to brake on the target.
--]]
function __refuelstop ()
   local target = ai.target()

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
      ai.popsubtask()
   end
end

