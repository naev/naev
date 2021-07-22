--[[
-- Basic tasks for a pilot, no need to reinvent the wheel with these.
--
-- Idea is to have it all here and only really work on the "control"
-- functions and such for each AI.
--]]


--[[
-- Helper function that checks to see if a value is in a table
--]]
function __intable( t, val )
   for k,v in ipairs(t) do
      if v==val then
         return true
      end
   end
   return false
end


--[[
-- Faces the target.
--]]
function __face( target )
   ai.face( target )
end
function __face_towards( target )
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
-- Move in zigzag around a direction
--]]
function __zigzag ( dir, angle )
   if mem.pm == nil then
      mem.pm = 1
   end

   if (mem.pm*dir < angle-20) or (mem.pm*dir > angle+25) then
      -- Orientation is totally wrong: reset timer
      ai.settimer(0, 2000)
   end

   if (mem.pm*dir < angle) then
      ai.turn(-mem.pm)
   else
      ai.turn(mem.pm)
      if (mem.pm*dir < angle+5) then -- Right orientation, wait for max vel
         --if ai.ismaxvel() then -- TODO : doesn't work well
         if ai.timeup(0) then
            mem.pm = -mem.pm
         end
      end
   end
   ai.accel()
end


--[[
-- Goes to a target position without braking
--]]
function __moveto_nobrake( target )
   local dir      = ai.face( target, nil, true )
   __moveto_generic( target, dir, false )
end


--[[
-- Goes to a target position without braking
--]]
function __moveto_nobrake_raw( target )
   local dir      = ai.face( target )
   __moveto_generic( target, dir, false )
end


--[[
-- Goes to a precise position.
--]]
function __moveto_precise ()
   local target   = ai.taskdata()
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
function moveto ()
   local target   = ai.taskdata()
   local dir      = ai.face( target, nil, true )
   __moveto_generic( target, dir, true )
end


--[[
-- Goes to a point in order to inspect (same as moveto, but pops when attacking)
--]]
function inspect_moveto( target )
   __moveto_nobrake( target )
end


--[[
-- moveto without velocity compensation.
--]]
function moveto_raw ()
   local target   = ai.taskdata()
   local dir      = ai.face( target )
   __moveto_generic( target, dir, true )
end


--[[
-- Generic moveto function.
--]]
function __moveto_generic( target, dir, brake )
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
   end
end


--[[
-- Follows it's target.
--]]
function follow ()
   local target = ai.taskdata()

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
   local target = ai.taskdata()
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
   local plt    = ai.pilot()
   local leader = plt:leader()

   if leader == nil or not leader:exists() then
      ai.poptask()
      return
   end

   if mem.form_pos == nil then -- Simply follow unaccurately
      local dir  = ai.face(leader)
      local dist = ai.dist(leader)
      if dist > 300 and dir < 10 then -- Must approach
         ai.accel()
      end

   else -- Ship has a precise position in formation
      if mem.app == nil then
         mem.app = 2
      end

      local angle, radius, method = table.unpack(mem.form_pos)
      local goal  = ai.follow_accurate(leader, radius, angle, mem.Kp, mem.Kd, method) -- Standard controller
      local dist  = ai.dist(goal)

      if mem.app == 2 then
         local dir   = ai.face(goal)
         if dist > 300 then
            if dir < 10 then  -- Must approach
               ai.accel()
            end
         else  -- Toggle precise positioning controller
            mem.app = 1
         end

      elseif mem.app == 1 then -- only small corrections to do
         if dist > 300 then -- We're much too far away, we need to toggle large correction
            mem.app = 2
         else  -- Derivative-augmented controller
            local goal0 = ai.follow_accurate(leader, radius, angle, 2*mem.Kp, 10*mem.Kd, method)
            local dist0 = ai.dist(goal0)
            local dir = ai.face(goal0)
            if dist0 > 300 then
               if dir < 10 then  -- Must approach
                  ai.accel()
               end
            else  -- No need to approach anymore
               mem.app = 0
            end
         end

      else
         local dir   = ai.face(goal)
         if dist > 300 then   -- Must approach
            mem.app = 1
         else   -- Face forward
            goal = plt:pos() + leader:vel()
            ai.face(goal)
         end
      end
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
function __hyperspace( target )
   hyperspace( target )
end
function __hyperspace_shoot( target )
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
   -- Shoot and approach
   __move_shoot()
   __hyp_approach()
end


function __land ()
   land()
end

function __land_shoot ()
   __choose_land_target ()
   ai.pushsubtask( "__landgo_shoot" )
end

function __landgo_shoot ()
   __move_shoot()
   __landgo()
end

function __move_shoot ()
   -- Shoot while going somewhere
   -- The difference with run_turret is that we pick a new enemy in this one
   if ai.hasturrets() then
      enemy = ai.getenemy()
      if enemy ~= nil then
         ai.weapset( 3 )
         ai.settarget( enemy )
         ai.shoot( true )
      end
   end
end


--[[
-- Attempts to land on a planet.
--]]
function __choose_land_target ()
   -- Only want to land once, prevents guys from never leaving.
   if mem.landed then
      ai.poptask()
      return
   end

   -- Set target if necessary
   local target = ai.taskdata()
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
         warn(string.format(_("Pilot '%s' tried to land with no landable assets!"),
               ai.pilot():name()))
         ai.poptask()
         return
      end
   end
end

function land ()
   __choose_land_target ()
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
         local p = ai.pilot()
         p:msg(p:followers(), "land", mem.land)
         ai.poptask() -- Done, pop task
      end
   end
end


--[[
-- Attempts to run away from the target.
--]]
function runaway ()

   -- Target must exist
   local target = ai.taskdata()
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
   local target = ai.taskdata()
   local plt    = ai.pilot()

   -- Target must exist
   if not target:exists() then
      ai.poptask()
      return true
   end

   -- Good to set the target for distress calls
   ai.settarget( target )

   -- See whether we have a chance to outrun the attacker
   local relspe = plt:stats().speed_max/target:stats().speed_max
   if plt:stats().mass <= 400 and relspe <= 1.01 and ai.hasprojectile() and (not ai.hasafterburner()) then
      -- Pilot is agile, but too slow to outrun the enemy: dodge
      local dir = ai.dir(target) + 180      -- Reverse (run away)
      if dir > 180 then dir = dir - 360 end -- Because of periodicity
      __zigzag(dir, 70)
   else
      ai.face(target, true)
      ai.accel()
   end

   -- Afterburner handling.
   if ai.hasafterburner() and plt:energy() > 10 then
      ai.weapset( 8, true )
   end

   return false
end
function __run_turret ()
   -- Shoot the target
   local target   = ai.taskdata()
   if target:exists() then
      ai.hostile(target)
      ai.settarget( target )
      local dist    = ai.dist(target)
      -- See if we have some turret to use
      if ai.hasturrets() then
         if dist < ai.getweaprange(3) then
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
   local jump     = ai.subtaskdata()
   local jdir
   local bdist    = ai.minbrakedist()
   local jdist    = ai.dist(jump)
   local plt      = ai.pilot()

   if jdist > bdist then

      local dozigzag = false
      if ai.taskdata():exists() then
         local relspe = plt:stats().speed_max/ai.taskdata():stats().speed_max
         if plt:stats().mass <= 400 and relspe <= 1.01 and ai.hasprojectile() and
            (not ai.hasafterburner()) and jdist > 3*bdist then
            dozigzag = true
         end
      end

      if dozigzag then
         -- Pilot is agile, but too slow to outrun the enemy: dodge
         local dir = ai.dir(jump)
         __zigzag(dir, 70)
      else
         if jdist > 3*bdist and plt:stats().mass < 600 then
            jdir = ai.careful_face(jump)
         else --Heavy ships should rush to jump point
            jdir = ai.face( jump, nil, true )
         end
         if jdir < 10 then
            ai.accel()
         end
      end
   else
      if ai.instantJump() then
         ai.pushsubtask( "__hyp_jump" )
      else
         ai.pushsubtask( "__run_hypbrake" )
      end
   end

   --Afterburner: activate while far away from jump
   if ai.hasafterburner() and plt:energy() > 10 then
      if jdist > 3 * bdist then
         ai.weapset( 8, true )
      else
         ai.weapset( 8, false )
      end
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
   local plt      = ai.pilot()

   if dist < bdist then -- Need to start braking
      ai.pushsubtask( "__landstop" )
   else

      local dozigzag = false
      if ai.taskdata():exists() then
         local relspe = plt:stats().speed_max/ai.taskdata():stats().speed_max
         if plt:stats().mass <= 400 and relspe <= 1.01 and ai.hasprojectile() and
            (not ai.hasafterburner()) and dist > 3*bdist then
            dozigzag = true
         end
      end

      if dozigzag then
         -- Pilot is agile, but too slow to outrun the enemy: dodge
         local dir = ai.dir(target)
         __zigzag(dir, 70)
      else

         -- 2 methods depending on mem.careful
         local dir
         if not mem.careful or dist < 3*bdist then
            dir = ai.face( target )
         else
            dir = ai.careful_face( target )
         end
         if dir < 10 then
            ai.accel()
         end
      end
   end

   --Afterburner
   if ai.hasafterburner() and plt:energy() > 10 then
      if dist > 3 * bdist then
         ai.weapset( 8, true )
      else
         ai.weapset( 8, false )
      end
   end

end


--[[
-- Starts heading away to try to hyperspace.
--]]
function hyperspace( target )
   if target == nil then
      target = ai.rndhyptarget()
      -- Can't jump so abort
      if target == nil then
         ai.poptask()
         return
      end
   end
   local pos = ai.sethyptarget(target)
   ai.pushsubtask( "__hyp_approach", pos )
end
function __hyp_approach ()
   local target   = ai.subtaskdata()
   local dir
   local dist     = ai.dist( target )
   local bdist    = ai.minbrakedist()

   -- 2 methods for dir
   if not mem.careful or dist < 3*bdist then
      dir = ai.face( target, nil, true )
   else
      dir = ai.careful_face( target )
   end

   -- Need to get closer
   if dir < 10 and dist > bdist then
      ai.accel()
   -- Need to start braking
   elseif dist < bdist then
      if ai.instantJump() then
         ai.pushsubtask("__hyp_jump")
      else
         ai.pushsubtask("__hyp_brake")
      end
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
      local p = ai.pilot()
      p:msg(p:followers(), "hyperspace", ai.nearhyptarget())
   end
   ai.popsubtask() -- Keep the task even if succeeding in case pilot gets pushed away.
end


--[[
-- Boards the target
--]]
function board ()
   local target = ai.taskdata()

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
   target = ai.taskdata()

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
   local target = ai.taskdata()

   -- make sure pilot exists
   if not target:exists() then
      ai.poptask()
      return
   end

   -- See if finished refueling
   if not ai.pilot():flags("refueling") then
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
   local target = ai.taskdata()

   -- make sure pilot exists
   if not target:exists() then
      ai.poptask()
      return
   end

   -- Set the target
   ai.settarget(target)

   -- See if finished refueling
   local p = ai.pilot()
   if not p:flags("refueling") then
      p:comm(target, _("Finished fuel transfer."))
      ai.poptask()

      -- Untarget
      ai.settarget( p )
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
   ai.weapset( 1 )
   local fieldNast = ai.taskdata()
   local field     = fieldNast[1]
   local ast       = fieldNast[2]
   local p         = ai.pilot()
   local wrange    = ai.getweaprange(nil, 0)
   local erange    = 100
   local trange    = math.min( math.max( erange, wrange * 3 / 4 ), wrange )
   local mbd       = ai.minbrakedist()

   -- See if there's a gatherable; if so, pop this task and gather instead
   local gat = ai.getgatherable( wrange )
   if gat ~= nil and ai.gatherablepos( gat ) ~= nil then
      ai.poptask()
      ai.pushtask("gather")
      return
   end

   ai.setasterotarget( field, ast )

   local target, vel = system.asteroidPos( field, ast )

   local dist, angle = vec2.polar( p:pos() - target )

   -- First task : place the ship close to the asteroid
   local goal = ai.face_accurate( target, vel, trange, angle, mem.Kp, mem.Kd )

   local dir  = ai.face(goal)
   local mod  = ai.dist(goal)

   if dir < 10 and mod > mbd then
      ai.accel()
   end

   local relpos = vec2.add( p:pos(), vec2.mul(target,-1) ):mod()
   local relvel = vec2.add( p:vel(), vec2.mul(vel,-1) ):mod()

   if relpos < wrange and relvel < 10 then
      ai.pushsubtask("__killasteroid")
   end
end
function __killasteroid ()
   local fieldNast = ai.taskdata()
   local field     = fieldNast[1]
   local ast       = fieldNast[2]
   local wrange    = ai.getweaprange()

   local target = system.asteroidPos( field, ast )
   local dir  = ai.face(target)

    -- See if there's a gatherable; if so, pop this task and gather instead
   local gat = ai.getgatherable( wrange )
   if gat ~= nil and ai.gatherablepos( gat ) ~= nil then
      ai.poptask()
      ai.pushtask("gather")
      return
   end

   -- Have to start over if we're out of range for some reason
   if ai.dist(target) > wrange then
      ai.poptask()
      return
   end

   -- Second task : destroy it
   if dir < 8 then
      ai.weapset( 1 )
      ai.shoot()
      ai.shoot(true)
   end
   if system.asteroidDestroyed( field, ast ) then
      ai.poptask()
      -- Last task : gather
      ai.pushtask("gather")
   end
end

--[[
-- Attempts to seek and gather gatherables
--]]
function gather ()
   if ai.pilot():cargoFree() == 0 then --No more cargo
      ai.poptask()
      return
   end

   local gat = ai.getgatherable( mem.gather_range )

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

   if dir < 10 and mod > 100 then
      ai.accel()
   end
end


-- Holds position
function hold ()
   if not ai.isstopped() then
      ai.brake()
   else
      ai.stop()
   end
end


-- Flies back and tries to either dock or stops when back at leader
function flyback( dock )
   local target = ai.pilot():leader()
   if not target or not target:exists() then
      ai.poptask()
      return
   end
   local goal = ai.follow_accurate(target, 0, 0, mem.Kp, mem.Kd)

   local dir  = ai.face( goal )
   local dist = ai.dist( goal )

   if dist > 300 then
      if dir < 10 then
         ai.accel()
      end
   else -- Time to dock
      if dock then
         ai.dock(target)
      else
         ai.poptask()
      end
   end
end


--[[
-- Checks to see if a pilot is visible
-- Assumes the pilot exists!
--]]
function __check_seeable( target )
   local self   = ai.pilot()
   if not target:flags("invisible") then
      -- Pilot still sees the target: continue attack
      if self:inrange( target ) then
         return true
      end

      -- Pilots on manual control (in missions or events) never loose target
      -- /!\ This is not necessary desirable all the time /!\
      -- TODO: there should probably be a flag settable to allow to outwit pilots under manual control
      if self:flags("manualcontrol") then
         return true
      end
   end
   return false
end


--[[
-- Aborts current task and tries to see what happened to the target.
--]]
function __investigate_target( target )
   local p = ai.pilot()
   ai.settarget(p) -- Un-target
   ai.poptask()
   -- Guess the pilot will be randomly between the current position and the
   -- future position if they go in the same direction with the same velocity
   local ttl = ai.dist(target) / p:stats().speed_max
   local fpos = target:pos() + vec2.newP( target:vel()*ttl, target:dir() ) * rnd.rnd()
   ai.pushtask("inspect_moveto", fpos )
end


--[[
-- Just loitering around.
--]]
function loiter( target )
   local dir      = ai.face( target )
   local dist     = ai.dist( target )
   if dist < 200 then
      ai.poptask()
      return
   end
   if dir < 10 then
      ai.accel()
   end
end


function __push_scan( target )
   -- Send a message if applicable
   local msg = _("Prepare to be scanned.")
   ai.pilot():comm( target, msg )
   ai.pushtask( "scan", target )
end


--[[
-- Tries to get close to scan the enemy
--]]
function scan( target )
   if not target:exists() then
      ai.poptask()
      return
   end

   -- Try to investigate if target lost
   if not __check_seeable( target ) then
      __investigate_target( target )
      return
   end

   -- Set target
   ai.settarget( target )
   local p = ai.pilot()

   -- Done scanning
   if ai.scandone() then -- Note this check MUST be done after settarget
      table.insert( mem.scanned, target )
      ai.poptask()
      if target:hasIllegal( p:faction() ) then
         ai.hostile( target )
         ai.pushtask( "attack", target )
         local msg = _("Illegal objects detected! Do not resist!")
         p:comm( target, msg )

         -- Make entire system hostile to player
         if target == player.pilot() then
            local f = p:faction()
            for k,v in ipairs(pilot.get(f)) do
               v:setHostile(true)
            end
            -- Do allies too :)
            for kf,vf in ipairs(f:allies()) do
               for k,v in ipairs(pilot.get(vf)) do
                  v:setHostile(true)
               end
            end

            -- Small faction hit
            f:modPlayer( -1 )
         end

         -- Have escorts attack
         for k,v in ipairs(p:followers()) do
            p:msg( v, "e_attack", target )
         end
      else
         local msg = _("Thank you for your cooperation.")
         p:comm( target, msg )

         -- Tell friends about the scanning
         local f = p:faction()
         for k,v in ipairs(pilot.get(f)) do
            p:msg( v, "scanned", target )
         end
         for kf,vf in ipairs(f:allies()) do
            for k,v in ipairs(pilot.get(vf)) do
               p:msg( v, "scanned", target )
            end
         end
      end
      return
   end

   -- Get stats about the enemy
   local dist = ai.dist(target)

   -- Get closer and scan
   ai.iface( target )
   if dist < 1000 then
      ai.accel()
   end
end


--[[
-- Check to see if a ship needs to be scanned.
--]]
function __needs_scan( target )
   if not mem.scanned then
      return false
   end
   for k,v in ipairs(mem.scanned) do
      if target==v then
         return false
      end
   end
   return true
end


--[[
-- Whether or not we want to scan, ignore players for now
--]]
function __wanttoscan( p, target )
   -- Don't care about stuff that doesn't need scan
   if not __needs_scan( target ) then
      return false
   end

   -- We always want to scan the player (no abusing allies)
   --[[
   if target == player.pilot() then
      return true
   end
   --]]

   -- Don't care about allies
   if ai.isally(target) then
      return false
   end

   return true
end


--[[
-- Tries to get find a good target to scan with some heuristics based on mass
-- and distance
--]]
function __getscantarget ()
   -- See if we should scan a pilot
   local p = ai.pilot()
   local pv = {}
   local inserted = {}
   for k,v in ipairs(p:getVisible()) do
      -- Only care about leaders
      local l = v:leader()
      if l and l:exists() then
         v = l
      end

      if not __intable( inserted, v ) then
         if __wanttoscan(p,v) then
            local d = ai.dist( v )
            local m = v:mass()
            table.insert( pv, {p=v, d=d, m=m} )
         end
         table.insert( inserted, v )
      end
   end
   inserted = nil
   -- We do a sort by distance and mass categories so that the AI will prefer
   -- larger ships before trying smaller ships. This is to avoid having large
   -- ships chasing after tiny ships
   local pm = p:mass()
   local pmh = pm * 1.5
   local pml = pm * 0.75
   table.sort( pv, function(a,b)
      if a.m > pmh and b.m > pmh then
         return a.d < b.d
      elseif a.m > pmh then
         return true
      elseif b.m > pmh then
         return false
      elseif a.m > pml and b.m > pml then
         return a.d < b.d
      elseif a.m > pml then
         return true
      elseif b.m > pml then
         return false
      else
         return a.d < b.d
      end
   end )

   if #pv==0 then
      return nil
   end
   return pv[1].p
end


function stealth( target )
   ai.brake()
   if ai.isstopped() then
      ai.stop()
   end
   -- TODO something to try to get them to restealth if failed, maybe move around?
   ai.stealth(true)
end


-- Delays the ship when entering systems so that it doesn't leave right away
function enterdelay ()
   if ai.timeup(0) then
      ai.pushtask("hyperspace")
   end
end
