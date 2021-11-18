--[[
-- Basic tasks for a pilot, no need to reinvent the wheel with these.
--
-- Idea is to have it all here and only really work on the "control"
-- functions and such for each AI.
--]]

--[[
-- Naming convention:

function foo( data ) -- normal task
function _bar( data ) -- subtask
local function __hoge( data ) -- internal use function
function __hoge( data ) -- internal in name only, or forward-declared local function ;)

-- Remark: the (sub)taskdata is passed as the (sub)task function argument
--]]

local fmt = require "format"

local __choose_land_target, __hyp_approach, __landgo, __moveto_generic, __run_target, __shoot_turret -- Forward-declared functions

--[[
-- Helper function that checks to see if a value is in a table
--]]
local function __intable( t, val )
   for k,v in ipairs(t) do
      if v==val then
         return true
      end
   end
   return false
end

--[[
-- Helper function to see if two pilots belong to the same fleet or not
--]]
function __sameFleet( pa, pb )
   local la = pa:leader()
   local lb = pa:leader()
   if not la or not la:exists() then la = pa end
   if not lb or not lb:exists() then lb = pb end
   return la == lb
end


--[[
-- Faces the target.
--]]
function face( target )
   ai.face( target )
end
function face_towards( target )
   local off = ai.face( target )
   if math.abs(off) < math.rad(5) then
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
function _subbrake ()
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

   local M_5DEG = math.pi/36
   if (mem.pm*dir < angle - 4*M_5DEG) or (mem.pm*dir > angle + 5*M_5DEG) then
      -- Orientation is totally wrong: reset timer
      ai.settimer(0, 2.0)
   end

   if (mem.pm*dir < angle) then
      ai.turn(-mem.pm)
   else
      ai.turn(mem.pm)
      if (mem.pm*dir < angle + M_5DEG) then -- Right orientation, wait for max vel
         --if ai.ismaxvel() then -- TODO : doesn't work well
         if ai.timeup(0) then
            mem.pm = -mem.pm
         end
      end
   end
   ai.accel()
end
local function __zigzag_run_decide( self, target )
   -- Some AI will not do fancy maneuvers
   if mem.simplecombat then return false end
   -- Try to figure it out
   local ss = self:stats()
   local relspe = ss.speed_max/target:stats().speed_max
   return ( ss.mass <= 400
            and relspe <= 1.01
            and ai.hasprojectile()
            and (not ai.hasafterburner() or self:energy() < 10)
          )
end


--[[
-- Goes to a target position without braking
--]]
function moveto_nobrake( target )
   local dir      = ai.face( target, nil, true )
   __moveto_generic( target, dir, false )
end


--[[
-- Goes to a target position without braking
--]]
function moveto_nobrake_raw( target )
   local dir      = ai.face( target )
   __moveto_generic( target, dir, false )
end


--[[
-- Goes to a precise position.
--]]
function moveto_precise( target )
   local dir      = ai.face( target, nil, true )
   local dist     = ai.dist( target )

   -- Handle finished
   if ai.isstopped() and dist < 10 then
      ai.poptask() -- Finished
      return
   end

   local bdist    = ai.minbrakedist()

   -- Need to get closer
   if dir < math.rad(10) and dist > bdist then
      ai.accel()

   -- Need to start braking
   elseif dist < bdist then
      ai.pushsubtask("_subbrake")
   end
end


--[[
-- Goes to a target position roughly
--]]
function moveto( target )
   local dir      = ai.face( target, nil, true )
   __moveto_generic( target, dir, true )
end


--[[
-- Goes to a point in order to inspect (same as moveto_nobrake, but pops when attacking)
--]]
function inspect_moveto( target )
   local dir      = ai.face( target, nil, true )
   __moveto_generic( target, dir, false )
end


--[[
-- moveto without velocity compensation.
--]]
function moveto_raw( target )
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
   if dir < math.rad(10) and dist > bdist then
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
function follow( target )
   -- Will just float without a target to escort.
   if not target:exists() then
      ai.poptask()
      return
   end

   local dir   = ai.face(target)
   local dist  = ai.dist(target)

   -- Must approach
   if dir < math.rad(10) and dist > 300 then
      ai.accel()
   end
end
function follow_accurate( target )
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
   if dir < math.rad(10) and mod > 300 then
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
      if dist > 300 and dir < math.rad(10) then -- Must approach
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
            if dir < math.rad(10) then  -- Must approach
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
               if dir < math.rad(10) then  -- Must approach
                  ai.accel()
               end
            else  -- No need to approach anymore
               mem.app = 0
            end
         end

      else
         ai.face(goal)
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
-- Tries to hyperspace asap.
--]]
function hyperspace_shoot( target )
   if target == nil then
      target = ai.rndhyptarget()
      if target == nil then
         ai.poptask()
         return
      else
         -- Push the new task
         ai.pushsubtask("hyperspace_shoot", target)
         return
      end
   end
   mem.target_bias = vec2.newP( rnd.rnd()*target:radius()/2, rnd.angle() )
   ai.pushsubtask( "_hyp_approach_shoot", target )
end
function _hyp_approach_shoot( target )
   -- Shoot and approach
   local enemy = ai.getenemy()
   __shoot_turret( enemy )
   __hyp_approach( target )
end

function land_shoot ( target )
   local planet = __choose_land_target ( target )
   ai.pushsubtask( "_landgo_shoot", planet )
end

function _landgo_shoot ( planet )
   local enemy = ai.getenemy()
   __shoot_turret( enemy )
   __landgo( planet )
end

--[[
-- Attempts to land on a planet.
--]]
function __choose_land_target ( target )
   -- Only want to land once, prevents guys from never leaving.
   if mem.landed then
      ai.poptask()
      return
   end

   -- Make sure target is valid
   if target == nil then
      local landplanet = ai.landplanet()
      if landplanet ~= nil then
         target = landplanet

      -- Bail out if no valid planet could be found.
      else
         warn(fmt.f(_("Pilot '{plt}' tried to land with no landable assets!"),
               {plt=ai.pilot():name()}))
         ai.poptask()
         return
      end
   end

   -- Decide exact land point
   mem.target_bias = vec2.newP( rnd.rnd()*target:radius()/2, rnd.angle() )

   return target
end

function land ( target )
   local planet = __choose_land_target ( target )
   ai.pushsubtask( "_landgo", planet )
end
function _landgo( planet )
   __landgo(planet)
end
function __landgo ( planet )
   local pl_pos = planet:pos() + mem.target_bias

   local dist     = ai.dist( pl_pos )
   local bdist    = ai.minbrakedist()

   -- 2 methods depending on mem.careful
   local dir
   if not mem.careful or dist < 3*bdist then
      dir = ai.face( pl_pos )
   else
      dir = ai.careful_face( pl_pos )
   end

   -- Need to get closer
   if dir < math.rad(10) and dist > bdist then
      ai.accel()

   -- Need to start braking
   elseif dist < bdist then
      ai.pushsubtask( "_landland", planet )
      ai.pushsubtask( "_subbrake" )
   end

end
function _landland ( planet )
   if not ai.land( planet ) then
      ai.popsubtask()
   else
      local p = ai.pilot()
      p:msg(p:followers(), "land", planet)
      ai.poptask() -- Done, pop task
   end
end


--[[
-- Attempts to run away from the target.
--]]
function runaway( target )
   -- Target must exist
   if not target or not target:exists() then
      ai.poptask()
      return
   end

   -- See if there's a target to use when running
   local t = ai.nearhyptarget()
   local p = ai.nearestplanet()

   if p == nil and t == nil then
      ai.pushsubtask( "_run_target" )
   elseif p == nil then
      mem.target_bias = vec2.newP( rnd.rnd()*t:radius()/2, rnd.angle() )
      ai.pushsubtask( "_run_hyp", {target, t} )
   elseif t == nil then
      mem.target_bias = vec2.newP( rnd.rnd()*p:radius()/2, rnd.angle() )
      ai.pushsubtask( "_run_landgo", {target, p} )
   else
      -- find which one is the closest
      local pilpos = ai.pilot():pos()
      local modt = vec2.mod(t:pos()-pilpos)
      local modp = vec2.mod(p:pos()-pilpos)
      if modt < modp then
         mem.target_bias = vec2.newP( rnd.rnd()*t:radius()/2, rnd.angle() )
         ai.pushsubtask( "_run_hyp", {target, t} )
      else
         mem.target_bias = vec2.newP( rnd.rnd()*p:radius()/2, rnd.angle() )
         ai.pushsubtask( "_run_landgo", {target, p} )
      end
   end
end
function runaway_nojump( target )
   if __run_target( target ) then return end
   __shoot_turret( target )
end
function runaway_jump( data )
   local t = data[2]
   mem.target_bias = vec2.newP( rnd.rnd()*t:radius()/2, rnd.angle() )
   ai.pushsubtask( "_run_hyp", data )
end
function runaway_land( data )
   local p = data[2]
   mem.target_bias = vec2.newP( rnd.rnd()*p:radius()/2, rnd.angle() )
   ai.pushsubtask( "_run_landgo", data )
end

function _run_target( target )
   __run_target( target )
end
function __run_target( target )
   local plt    = ai.pilot()

   -- Target must exist
   if not target or not target:exists() then
      ai.poptask()
      return true
   end

   -- Good to set the target for distress calls
   ai.settarget( target )

   -- See whether we have a chance to outrun the attacker
   if __zigzag_run_decide( plt, target ) then
      -- Pilot is agile, but too slow to outrun the enemy: dodge
      local dir = ai.dir(target) + math.pi      -- Reverse (run away)
      if dir > math.pi then dir = dir - 2*math.pi end -- Because of periodicity
      __zigzag(dir, math.rad(70))
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
function __shoot_turret( target )
   -- Target must exist
   if not target or not target:exists() then
      return
   end

   -- Shoot the target
   ai.hostile(target)
   ai.settarget(target)
   local dist = ai.dist(target)
   -- See if we have some turret to use
   if ai.hasturrets() then
      if dist < ai.getweaprange(3) then
         ai.weapset( 3 )
         ai.shoot( true )
         ai.weapset( 9 )
      end
   end
end
function _run_hyp( data )
   local enemy  = data[1]
   local jump   = data[2]
   local jp_pos = jump:pos() + mem.target_bias

   -- Shoot the target
   __shoot_turret( enemy )

   -- Go towards jump
   local jdir
   local bdist    = ai.minbrakedist()
   local jdist    = ai.dist(jp_pos)
   local plt      = ai.pilot()

   if jdist > bdist then

      local dozigzag = false
      if enemy:exists() then
         if __zigzag_run_decide( plt, enemy ) and jdist > 3*bdist then
            dozigzag = true
         end
      end

      if dozigzag then
         -- Pilot is agile, but too slow to outrun the enemy: dodge
         local dir = ai.dir(jp_pos)
         __zigzag(dir, math.rad(70))
      else
         if jdist > 3*bdist and plt:stats().mass < 600 then
            jdir = ai.careful_face(jp_pos)
         else --Heavy ships should rush to jump point
            jdir = ai.face( jp_pos, nil, true )
         end
         if jdir < math.rad(10) then
            ai.accel()
         end
      end
   else
      if ai.instantJump() then
         ai.pushsubtask( "_hyp_jump", jump )
      else
         ai.pushsubtask( "_hyp_jump", jump )
         ai.pushsubtask("_subbrake")
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

function _run_landgo( data )
   local enemy  = data[1]
   local planet = data[2]
   local pl_pos = planet:pos() + mem.target_bias

   -- Shoot the target
   __shoot_turret( enemy )

   local dist     = ai.dist( pl_pos )
   local bdist    = ai.minbrakedist()
   local plt      = ai.pilot()

   if dist < bdist then -- Need to start braking
      ai.pushsubtask( "_landland", planet )
      ai.pushsubtask( "_subbrake" )
      ai.weapset( 8, false ) -- Turn off afterburner just in case
      return -- Don't try to afterburn

   else
      local dozigzag = false
      if enemy:exists() then
         if __zigzag_run_decide( plt, enemy ) and dist > 3*bdist then
            dozigzag = true
         end
      end

      if dozigzag then
         -- Pilot is agile, but too slow to outrun the enemy: dodge
         local dir = ai.dir(pl_pos)
         __zigzag(dir, math.rad(70))
      else

         -- 2 methods depending on mem.careful
         local dir
         if not mem.careful or dist < 3*bdist then
            dir = ai.face( pl_pos )
         else
            dir = ai.careful_face( pl_pos )
         end
         if dir < math.rad(10) then
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
-- Tries to return to the lane, shooting at nearby enemies if necessary
--]]
function return_lane( data )
   local enemy = data[1]
   local target = data[2]
   __shoot_turret(enemy)
   moveto_nobrake( target )
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
      else
         -- Push the new task
         ai.pushsubtask("hyperspace", target)
         return
      end
   end
   mem.target_bias = vec2.newP( rnd.rnd()*target:radius()/2, rnd.angle() )
   ai.pushsubtask( "_hyp_approach", target )
end
function _hyp_approach( target )
   __hyp_approach( target )
end
function __hyp_approach( target )
   local dir
   local pos      = target:pos() + mem.target_bias
   local dist     = ai.dist( pos )
   local bdist    = ai.minbrakedist()

   -- 2 methods for dir
   if not mem.careful or dist < 3*bdist then
      dir = ai.face( pos, nil, true )
   else
      dir = ai.careful_face( pos )
   end

   -- Need to get closer
   if dir < math.rad(10) and dist > bdist then
      ai.accel()
   -- Need to start braking
   elseif dist < bdist then
      if ai.instantJump() then
         ai.pushsubtask("_hyp_jump", target)
      else
         ai.pushsubtask("_hyp_jump", target)
         ai.pushsubtask("_subbrake")
      end
   end
end
function _hyp_jump ( jump )
   if ai.hyperspace( jump ) == nil then
      local p = ai.pilot()
      p:msg(p:followers(), "hyperspace", jump)
   end
   ai.popsubtask() -- Keep the task even if succeeding in case pilot gets pushed away.
end


--[[
-- Boards the target
--]]
function board( target )
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
      ai.pushsubtask( "_boardstop", target )
   elseif dir < math.rad(10) then
      ai.accel()
   end
end


--[[
-- Attempts to brake on the target.
--]]
function _boardstop( target )
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
         mem.boarded = mem.boarded or 0
         mem.boarded = mem.boarded + 1
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
function refuel( target )
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
      ai.pushsubtask( "_refuelstop", target )
   elseif dir < math.rad(10) then
      ai.accel()
   end
end

--[[
-- Attempts to brake on the target.
--]]
function _refuelstop( target )
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
function mine( fieldNast )
   ai.weapset( 1 )
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

   local _dist, angle = vec2.polar( p:pos() - target )

   -- First task : place the ship close to the asteroid
   local goal = ai.face_accurate( target, vel, trange, angle, mem.Kp, mem.Kd )

   local dir  = ai.face(goal)
   local mod  = ai.dist(goal)

   if dir < math.rad(10) and mod > mbd then
      ai.accel()
   end

   local relpos = vec2.add( p:pos(), vec2.mul(target,-1) ):mod()
   local relvel = vec2.add( p:vel(), vec2.mul(vel,-1) ):mod()

   if relpos < wrange and relvel < 10 then
      ai.pushsubtask("_killasteroid", fieldNast )
   end
end
function _killasteroid( fieldNast )
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
   if dir < math.rad(8) then
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

   if dir < math.rad(10) and mod > 100 then
      ai.accel()
   end
end


-- Holds position
function hold ()
   follow_fleet ()
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
      if dir < math.rad(10) then
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

   -- No need to investigate: target has jumped out.
   if target:flags("jumpingout") then
      return
   end

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
   local dir   = ai.face( target, nil, true )
   local dist  = ai.dist( target )
   if dist < 200 then
      ai.poptask()
      return
   end
   if dir < math.rad(10) then
      ai.accel()
   end
end
-- Last vertex, so poptask when at brakedistance
function loiter_last( target )
   local dir   = ai.face( target, nil, true )
   local dist  = ai.dist( target )
   local bdist = ai.minbrakedist()
   if dist < bdist then
      ai.poptask()
      return
   end
   if dir < math.rad(10) then
      ai.accel()
   end
end


function __push_scan( target )
   -- Send a message if applicable
   local msg = mem.scan_msg or _("Prepare to be scanned.")
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
         local msg = mem.scan_msg_bad or _("Illegal objects detected! Do not resist!")
         p:comm( target, msg )

         -- Player gets faction hit and more hostiles on them
         if target == player.pilot() then
            for k,v in ipairs(p:getAllies(5000, nil, true, false, true)) do
               v:setHostile(true)
            end
            -- Small faction hit
            p:faction():modPlayer( -1 )
         end
      else
         local msg = mem.scan_msg_ok or _("Thank you for your cooperation.")
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
   if dist > 1000 then
      ai.accel()
   end
end


--[[
-- Check to see if a ship needs to be scanned.
--]]
local function __needs_scan( target )
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
local function __wanttoscan( _p, target )
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

   -- Don't care about allies nor enemies (should attack instead)
   if ai.isally(target) or ai.isenemy(target) then
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
   do
      local inserted = {}
      for k,v in ipairs(p:getVisible()) do
         -- Only care about leaders
         local l = v:leader()
         if l and l:exists() then
            v = l
         end

         if not __intable( inserted, v ) then
            if __wanttoscan( p, v ) then
               local d = ai.dist2( v )
               local m = v:mass()
               table.insert( pv, {p=v, d=d, m=m} )
            end
            table.insert( inserted, v )
         end
      end
   end
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


function stealth( _target )
   ai.brake()
   if ai.isstopped() then
      ai.stop()
   end
   -- TODO something to try to get them to restealth if failed, maybe move around?
   ai.stealth(true)
end


function ambush_moveto( target )
   -- Make sure stealthed
   if not ai.stealth(true) then
      ai.poptask()
      return
   end
   local dir = ai.face( target, nil, true )
   local dist = ai.dist( target )
   if dir < math.rad(10) then
      ai.accel()
   end
   if dist < 300 then
      ai.poptask()
   end
end


function ambush_stalk( target )
   -- Make sure stealthed or attack
   if not ai.stealth(true) then
      ai.poptask()
      ai.pushtask( "attack", target )
      return
   end

   if not target:exists() then
      ai.poptask()
      return
   end

   -- Try to accurately face them
   local dir   = ai.face(target, nil, true)
   local dist  = ai.dist(target)

   -- Must approach
   local range = ai.getweaprange( 3 )
   if dist < range * mem.atk_aim then
      -- Go for the kill!
      ai.pushtask( "attack", target )
   elseif dir < math.rad(10) and dist > 300 then
      ai.accel()
   end
end


-- Delays the ship when entering systems so that it doesn't leave right away
function enterdelay ()
   if ai.timeup(0) then
      ai.pushtask("hyperspace")
   end
end

function idle_wait ()
   if ai.timeup(0) then
      ai.poptask()
   end
end
