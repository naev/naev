--[[
-- Basic tasks for a pilot, no need to reinvent the wheel with these.
--
-- Idea is to have it all here and only really work on the "control"
-- functions and such for each AI.
--]]


--[[
-- Faces the target.
--]]
function __face ()
   local target = ai.target()
   ai.face( target )
end
function __face_towards ()
   local target = ai.target()
   local off = ai.face( target )
   if math.abs(off) < 5 then
      ai.poptask()
   end
end


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
-- Brakes the ship
--]]
function __subbrake ()
   ai.brake()
   if ai.isstopped() then
      ai.stop()
      ai.popsubtask()
      return
   end
end


--[[
-- Goes to a target position without braking
--]]
function __goto_nobrake ()
   local target   = ai.target()
   local dir      = ai.face( target, nil, true )
   __goto_generic( target, dir, false )
end


--[[
-- Goes to a target position without braking
--]]
function __goto_nobrake_raw ()
   local target   = ai.target()
   local dir      = ai.face( target )
   __goto_generic( target, dir, false )
end


--[[
-- Goes to a precise position.
--]]
function __goto_precise ()
   local target   = ai.target()
   local dir      = ai.face( target, nil, true )
   local dist     = ai.dist( target )

   -- Handle finished
   if ai.isstopped() and dist < 10 then
      ai.poptask() -- Finished
      return
   end

   local bdist    = ai.minbrakedist()

   -- Need to get closer
   if dir < 10 and dist > bdist then
      ai.accel()

   -- Need to start braking
   elseif dist < bdist then
      ai.pushsubtask("__subbrake")
   end
end




--[[
-- Goes to a target position roughly
--]]
function goto ()
   local target   = ai.target()
   local dir      = ai.face( target, nil, true )
   __goto_generic( target, dir, true )
end


--[[
-- Goto without velocity compensation.
--]]
function goto_raw ()
   local target   = ai.target()
   local dir      = ai.face( target )
   __goto_generic( target, dir, true )
end


--[[
-- Generic GOTO function.
--]]
function __goto_generic( target, dir, brake, subtask )
   local dist     = ai.dist( target )
   local bdist
   if brake then
      bdist    = ai.minbrakedist()
   else
      bdist    = 50
   end

   -- Need to get closer
   if dir < 10 and dist > bdist then
      ai.accel()

   -- Need to start braking
   elseif dist < bdist then
      ai.poptask()
      if brake then
         ai.pushtask("brake")
      end
      return
   end
end


--[[
-- Follows it's target.
--]]
function follow ()
   local target = ai.target()
 
   -- Will just float without a target to escort.
   if not target:exists() then
      ai.poptask()
      return
   end

   local dir   = ai.face(target)
   local dist  = ai.dist(target)
 
   -- Must approach
   if dir < 10 and dist > 300 then
      ai.accel()
 
   end
end
function follow_accurate ()
   local target = ai.target()
   local p = ai.pilot()
 
   -- Will just float without a target to escort.
   if not target:exists() then
      ai.poptask()
      return
   end

   local goal = ai.follow_accurate(target, mem.radius, 
         mem.angle, mem.Kp, mem.Kd)

   local mod = vec2.mod(goal - p:pos())

   --  Always face the goal
   local dir   = ai.face(goal)

   if dir < 10 and mod > 300 then
      ai.accel()
   end

end

-- Default action for non-leader pilot in fleet
function follow_fleet ()
   local leader = ai.pilot():leader()
 
   if leader == nil or not leader:exists() then
      ai.poptask()
      return
   end

   local goal = leader
   if mem.form_pos ~= nil then
      local angle, radius, method = unpack(mem.form_pos)
      goal = ai.follow_accurate(leader, radius, angle, mem.Kp, mem.Kd, method)
   end

   
   local dir   = ai.face(goal)
   local dist  = ai.dist(goal)
 
   -- Must approach
   if dir < 10 and dist > 300 then
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
-- Runaway without jumping
--]]
function __runaway_nojump ()
   runaway_nojump()
end


--[[
-- Tries to hyperspace asap.
--]]
function __hyperspace ()
   hyperspace()
end
function __hyperspace_shoot ()
   local target = ai.target()
   if target == nil then
      target = ai.rndhyptarget()
      if target == nil then
         return
      end
   end
   local pos = ai.sethyptarget(target)
   ai.pushsubtask( "__hyp_approach_shoot", pos )
end
function __hyp_approach_shoot ()
   -- Shoot
   if ai.hasturrets() then
      enemy = ai.getenemy()
      if enemy ~= nil then
         ai.weapset( 3 )
         ai.settarget( enemy )
         ai.shoot( true )
      end
   end
   __hyp_approach()
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
      local landplanet = ai.landplanet()
      if landplanet ~= nil then
         mem.land = landplanet

      -- Bail out if no valid planet could be found.
      else
         warn(string.format("Pilot '%s' tried to land with no landable assets!",
               ai.pilot():name()))
         ai.poptask()
         return
      end
   end

   ai.pushsubtask( "__landgo" )
end
function __landgo ()
   local target   = mem.land
   
   local dist     = ai.dist( target )
   local bdist    = ai.minbrakedist()

   -- 2 methods depending on mem.careful
   local dir
   if not mem.careful or dist < 3*bdist then
      dir = ai.face( target )
   else
      dir = ai.careful_face( target )
   end

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
      else
         ai.pilot():msg(ai.pilot():followers(), "land")
         ai.poptask() -- Done, pop task
      end
   end
end


--[[
-- Attempts to run away from the target.
--]]
function runaway ()

   -- Target must exist
   local target = ai.target()
   if not target:exists() then
      ai.poptask()
      return
   end

   -- See if there's a target to use when running
   local t = ai.nearhyptarget()
   local p = ai.nearestplanet()

   if p == nil and t == nil then
      ai.pushsubtask( "__run_target" )
   elseif p == nil then
      local pos = ai.sethyptarget(t)
      ai.pushsubtask( "__run_hyp", pos )
   elseif t == nil then
      mem.land = p:pos()
      ai.pushsubtask( "__landgo" )
   else 
      -- find which one is the closest
      local pilpos = ai.pilot():pos()
      local modt = vec2.mod(t:pos()-pilpos) 
      local modp = vec2.mod(p:pos()-pilpos)
      if modt < modp then
         local pos = ai.sethyptarget(t)
         ai.pushsubtask( "__run_hyp", pos )
      else
         mem.land = p:pos()
         ai.pushsubtask( "__run_landgo" )
      end
   end
end
function runaway_nojump ()
   if __run_target() then return end
   __run_turret()
end
function __run_target ()
   local target = ai.target()

   -- Target must exist
   if not target:exists() then
      ai.poptask()
      return true
   end

   -- Good to set the target for distress calls
   ai.settarget( target )

   local dir   = ai.face(target, true)
   ai.accel()

   -- Afterburner handling.         
   if ai.hasafterburner() and ai.pilot():energy() > 10 then
      ai.weapset( 8, true )
   end

   return false
end
function __run_turret ()
   -- Shoot the target
   local target   = ai.target()
   if target:exists() then
      ai.hostile(target)
      ai.settarget( target )
      local dist    = ai.dist(target)
      -- See if we have some turret to use
      if ai.hasturrets() then
         if dist < ai.getweaprange(true) then
            ai.weapset( 3 )
            ai.shoot( true )
         end
      end
   end
end
function __run_hyp ()
   -- Shoot the target
   __run_turret()

   -- Go towards jump
   local jump     = ai.subtarget()
   local jdir
   local bdist    = ai.minbrakedist()
   local jdist    = ai.dist(jump)

   if jdist > 3*bdist and ai.pilot():stats().mass < 600 then
      jdir = ai.careful_face(jump)
   else --Heavy ships should rush to jump point
      jdir = ai.face(jump)
   end
   
   --Afterburner: activate while far away from jump
   if ai.hasafterburner() and ai.pilot():energy() > 10 then
      if jdist > 3 * bdist then
         ai.weapset( 8, true )
      else
         ai.weapset( 8, false )
      end
   end
   if jdist > bdist and jdir < 10 then       
      ai.accel()
   elseif jdist < bdist then
      ai.pushsubtask( "__run_hypbrake" )
   end
end
function __run_hypbrake ()

   -- The braking
   ai.brake()
   if ai.isstopped() then
      ai.stop()
      ai.popsubtask()
      ai.pushsubtask( "__hyp_jump" )
   end
end

function __run_landgo ()
   -- Shoot the target
   __run_turret()

   local target   = mem.land
   local dist     = ai.dist( target )
   local bdist    = ai.minbrakedist()

   -- 2 methods depending on mem.careful
   local dir
   if not mem.careful or dist < 3*bdist then
      dir = ai.face( target )
   else
      dir = ai.careful_face( target )
   end

   --Afterburner
   if ai.hasafterburner() and ai.pilot():energy() > 10 then
      if dist > 3 * bdist then
         ai.weapset( 8, true )
      else
         ai.weapset( 8, false )
      end
   end

   -- Need to get closer
   if dir < 10 and dist > bdist then
      ai.accel()

   -- Need to start braking
   elseif dist < bdist then
      ai.pushsubtask( "__landstop" )
   end

end


--[[
-- Starts heading away to try to hyperspace.
--]]
function hyperspace ()
   local target = ai.target()
   if target == nil then
      target = ai.rndhyptarget()
      if target == nil then
         return
      end
   end
   local pos = ai.sethyptarget(target)
   ai.pushsubtask( "__hyp_approach", pos )
end
function __hyp_approach ()
   local target   = ai.subtarget()
   local dir
   local dist     = ai.dist( target )
   local bdist    = ai.minbrakedist()

   -- 2 methods for dir
   if not mem.careful or dist < 3*bdist then
      dir = ai.face( target )
   else
      dir = ai.careful_face( target )
   end

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
      ai.pilot():msg(ai.pilot():followers(), "hyperspace", ai.nearhyptarget())
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
   if not target:exists() then
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
   if not target:exists() then
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
   if not target:exists() then
      ai.poptask()
      return
   end

   -- See if finished refueling
   if not ai.pilot():flags().refueling then
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
   if not target:exists() then
      ai.poptask()
      return
   end

   -- Set the target
   ai.settarget(target)

   -- See if finished refueling
   if not ai.pilot():flags().refueling then
      ai.pilot():comm(target, "Finished fuel transfer.")
      ai.poptask()

      -- Untarget
      ai.settarget( ai.pilot() )
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

--[[
-- Mines an asteroid
--]]
function mine ()
   local fieldNast = ai.target()
   local field     = fieldNast[1]
   local ast       = fieldNast[2]
   local p         = ai.pilot()

   ai.setasterotarget( field, ast )

   local target, vel = system.asteroidpos( field, ast )
   local target2 = vec2.add(target,200,0)

   -- First task : place the ship close to the asteroid
   local goal = ai.face_accurate( target2, vel, 0, 0, mem.Kp, mem.Kd )

   local dir  = ai.face(goal)
   local mod  = ai.dist(goal)

   if dir < 10 and mod > 300 then
      ai.accel()
   end
   local relpos = vec2.add( p:pos(), vec2.mul(target2,-1) ):mod()
   local relvel = vec2.add( p:vel(), vec2.mul(vel,-1) ):mod()
   -- TODO : make 30 and 2 parameters dependent to Kp and Kd
   if relpos < 30 and relvel < 2 then
      ai.pushsubtask("__killasteroid")
   end
end
function __killasteroid ()
   local fieldNast = ai.target()
   local field     = fieldNast[1]
   local ast       = fieldNast[2]

   local target = system.asteroidpos( field, ast )
   local dir  = ai.face(target)

   -- Second task : destroy it
   if dir < 8 then
      ai.weapset( 3 )
      ai.shoot()
   end
   if system.asteroiddestroyed( field, ast ) then
      ai.poptask()
      -- Last task : gather
      ai.pushtask("gather")
   end
end

--[[
-- Attepts to seek and gather gatherables
--]]
function gather ()
   if ai.pilot():cargoFree() == 0 then --No more cargo
      ai.poptask()
      return
   end

   local gat = ai.getgatherable( 500 );

   if gat == nil then -- Nothing to gather
      ai.poptask()
      return
   end

   local target, vel = ai.gatherablepos( gat )
   if target == nil then -- gatherable disappeared
      ai.poptask()
      return
   end

   local goal = ai.face_accurate( target, vel, 0, 0, mem.Kp, mem.Kd )

   local dir  = ai.face(goal)
   local mod  = ai.dist(goal)

   if dir < 10 and mod > 300 then
      ai.accel()
   end
end

