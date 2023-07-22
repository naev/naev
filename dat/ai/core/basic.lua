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
local atk = require "ai.core.attack.util"
local fmt = require "format"
local scans = require "ai.core.misc.scans"

local __choose_land_target, __hyp_approach, __landgo, __moveto_generic, __run_target, __shoot_turret -- Forward-declared functions

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
-- luacheck: globals _subbrake (AI Task functions passed by name)
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
local function __zigzag ( dir, angle )
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
-- Zig zags towards the target
--]]
-- luacheck: globals _attack_zigzag (AI Task functions passed by name)
function _attack_zigzag( target )
   target = atk.com_think( target )
   if target == nil then return end
   if target:flags("disabled") then
      ai.popsubtask()
      return
   end

   -- See if the enemy is still seeable
   if not atk.check_seeable( target ) then return end

   ai.settarget(target)

   -- Is there something to dodge?
   if not ai.hasprojectile() then
      ai.popsubtask()
      return
   end

   -- Are we ready to shoot?
   local dist = ai.dist( target )
   local range = ai.getweaprange(3)
   if dist < range then
      ai.popsubtask()
      return
   end

   local dir = ai.dir( target )
   __zigzag(dir, math.pi/6)
end


--[[
-- Goes to a target position without braking
--]]
function moveto_nobrake( target )
   local dir   = ai.face( target, nil, true )
   __moveto_generic( target, dir )
end


--[[
-- Goes to a target position without braking
--]]
function moveto_nobrake_raw( target )
   local dir   = ai.face( target )
   __moveto_generic( target, dir )
end


--[[
-- Goes to a target position
--]]
function moveto( target )
   local dir   = ai.face( target, nil, true )
   local dist  = ai.dist( target )

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
-- Goes to a point in order to inspect (same as moveto_nobrake, but pops when attacking)
--]]
-- luacheck: globals inspect_moveto (AI Task functions passed by name)
function inspect_moveto( target )
   local dir   = ai.face( target, nil, true )
   __moveto_generic( target, dir )
end


--[[
-- Lunges towards the target always thrusting
--]]
function lunge( target )
   if not target:exists() then
      ai.poptask()
      return
   end
   ai.aim( target )
   ai.accel()
end


--[[
-- Generic moveto function.
--]]
function __moveto_generic( target, dir )
   local dist  = ai.dist( target )
   local bdist = 50

   -- Need to start braking
   if dist < bdist then
      ai.poptask()

   -- Need to get closer
   elseif dir < math.rad(10) and dist > bdist then
      ai.accel()
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

   -- Stealth like whoever is being followed
   ai.stealth( target:flags("stealth") )

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

   -- Stealth like whoever is being followed
   ai.stealth( target:flags("stealth") )

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
-- luacheck: globals follow_fleet (AI Task functions passed by name)
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

-- luacheck: globals _hyp_approach_shoot (AI Task functions passed by name)
function _hyp_approach_shoot( target )
   -- Shoot and approach
   local enemy = atk.preferred_enemy()
   __shoot_turret( enemy )
   __hyp_approach( target )
end

function land_shoot ( target )
   local planet = __choose_land_target ( target )
   ai.pushsubtask( "_landgo_shoot", planet )
end

-- luacheck: globals _landgo_shoot (AI Task functions passed by name)
function _landgo_shoot ( planet )
   local enemy = atk.preferred_enemy()
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
      local landspob = ai.landspob()
      if landspob ~= nil then
         target = landspob

      -- Bail out if no valid planet could be found.
      else
         warn(fmt.f(_("Pilot '{plt}' tried to land with no landable spob!"), {plt=ai.pilot()}))
         ai.poptask()
         return
      end
   end

   -- Decide exact land point
   mem.target_bias = vec2.newP( rnd.rnd()*target:radius()/2, rnd.angle() )

   return target
end

function land( target )
   local planet = __choose_land_target ( target )
   ai.pushsubtask( "_landgo", planet )
end

-- luacheck: globals _landgo (AI Task functions passed by name)
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

-- luacheck: globals _landland (AI Task functions passed by name)
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
   if mem.mothership and mem.mothership:exists() then
      local goal = ai.follow_accurate( mem.mothership, 0, 0, mem.Kp, mem.Kd )
      local dir  = ai.face( goal )
      local dist = ai.dist( goal )

      if dist > 300 then
         if dir < math.rad(10) then
            ai.accel()
         end
      else -- Time to dock
         ai.dock( mem.mothership )
      end
      return
   end

   -- Target must exist
   if not target or not target:exists() then
      ai.poptask()
      return
   end

   -- See if there's a target to use when running
   local t = ai.nearhyptarget()
   local p = ai.nearestspob()

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

-- luacheck: globals _run_target (AI Task functions passed by name)
function _run_target( target )
   __run_target( target )
end
function __run_target( target )
   local plt = ai.pilot()

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
   if mem._o then
      if mem._o.blink_drive then
         plt:outfitToggle( mem._o.blink_drive, true )
      elseif mem._o.blink_engine then
         plt:outfitToggle( mem._o.blink_engine, true )
      end
   end

   return false
end
function __shoot_turret( target )
   -- Target must exist
   if not target or not target:exists() then
      return
   end

   -- Always launch fighters while running away
   ai.weapset( 5 )

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

-- luacheck: globals _run_hyp (AI Task functions passed by name)
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
         if jdist > 3*bdist and plt:mass() < 600 then
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
   -- Hyperbolic blink drives have a distance of 2000
   if mem._o then
      if mem._o.blink_drive and jdist > 500 + 3 * bdist then
         plt:outfitToggle( mem._o.blink_drive, true )
      elseif mem._o.blink_engine and jdist > 2000 + 3 * bdist then
         plt:outfitToggle( mem._o.blink_engine, true )
      end
   end
end

-- luacheck: globals _run_landgo (AI Task functions passed by name)
function _run_landgo( data )
   local enemy  = data[1]
   local planet = data[2]
   local pl_pos = planet:pos() + mem.target_bias

   -- Shoot the target
   __shoot_turret( enemy )

   local dir
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
         dir = ai.dir(pl_pos)
         __zigzag(dir, math.rad(70))
      else

         -- 2 methods depending on mem.careful
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
   -- Hyperbolic blink drives have a distance of 2000
   if mem._o and dir < math.rad(25) then
      if mem._o.blink_drive and dist > 500 + 3 * bdist then
         plt:outfitToggle( mem._o.blink_drive, true )
      elseif mem._o.blink_engine and dist > 2000 + 3 * bdist then
         plt:outfitToggle( mem._o.blink_engine, true )
      end
   end
end


--[[
-- Tries to return to the lane, shooting at nearby enemies if necessary
--]]
-- luacheck: globals return_lane (AI Task functions passed by name)
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

-- luacheck: globals _hyp_approach (AI Task functions passed by name)
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

-- luacheck: globals _hyp_jump (AI Task functions passed by name)
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
-- luacheck: globals board (AI Task functions passed by name)
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
-- luacheck: globals _boardstop (AI Task functions passed by name)
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
-- luacheck: globals _refuelstop (AI Task functions passed by name)
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
-- luacheck: globals mine (AI Task functions passed by name)
function mine( ast )
   if mem._o and mem._o.plasma_drill then
      ai.pushsubtask("mine_drill", ast)
   else
      ai.pushsubtask("mine_shoot", ast)
   end
end
-- luacheck: globals mine_drill (AI Task functions passed by name)
function mine_drill( ast )
   if not ast:exists() then
      ai.poptask()
      return
   end

   local p         = ai.pilot()
   local mbd       = ai.minbrakedist()

   ai.setasterotarget( ast )

   local target = ast:pos()
   local vel = ast:vel()
   local _dist, angle = vec2.polar( p:pos() - target )

   -- First task : place the ship close to the asteroid
   local goal = ai.face_accurate( target, vel, 0, angle, mem.Kp, mem.Kd )

   local dir  = ai.face(goal)
   local mod  = ai.dist(goal)

   if dir < math.rad(10) and mod > mbd then
      ai.accel()
   elseif mod < mbd then
      ai.pushsubtask( "mine_drill_brake", ast )
   end
end
-- luacheck: globals mine_drill_brake (AI Task functions passed by name)
function mine_drill_brake( ast )
   if ai.isstopped() then
      ai.popsubtask()
      return
   end
   ai.setasterotarget( ast )
   ai.brake()
   ai.pilot():outfitToggle( mem._o.plasma_drill, true )
end
-- luacheck: globals mine_shoot (AI Task functions passed by name)
function mine_shoot( ast )
   if not ast:exists() then
      ai.poptask()
      return
   end

   ai.weapset( 1 )
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

   ai.setasterotarget( ast )

   local target = ast:pos()
   local vel = ast:vel()
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
      ai.pushsubtask("_killasteroid", ast )
   end
end

-- luacheck: globals _killasteroid (AI Task functions passed by name)
function _killasteroid( ast )
   local wrange    = ai.getweaprange()

   local target = ast:pos()
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
   if not ast:exists() then
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
-- luacheck: globals hold (AI Task functions passed by name)
function hold ()
   follow_fleet ()
end


-- Flies back and tries to either dock or stops when back at leader
-- luacheck: globals flyback (AI Task functions passed by name)
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
-- Just loitering around.
--]]
-- luacheck: globals loiter (AI Task functions passed by name)
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
-- luacheck: globals loiter_last (AI Task functions passed by name)
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


--[[
-- Tries to get close to scan the enemy
--]]
-- luacheck: globals scan (AI Task functions passed by name)
function scan( target )
   scans.scan( target )
end


function stealth( _target )
   ai.brake()
   if ai.isstopped() then
      ai.stop()
   end
   -- TODO something to try to get them to restealth if failed, maybe move around?
   ai.stealth(true)
end


-- luacheck: globals ambush_moveto (AI Task functions passed by name)
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


-- luacheck: globals ambush_stalk (AI Task functions passed by name)
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
-- luacheck: globals enterdelay (AI Task functions passed by name)
function enterdelay ()
   if ai.timeup(0) then
      ai.pushtask("hyperspace")
   end
end

-- luacheck: globals idle_wait (AI Task functions passed by name)
function idle_wait ()
   if ai.timeup(0) then
      ai.poptask()
   end
end

-- luacheck: globals jumpin_wait (AI Task functions passed by name)
function jumpin_wait ()
   if ai.timeup(0) then
      ai.poptask()
   end
end
