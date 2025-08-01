local fmt = require "format"
local lanes = require "ai.core.misc.lanes"
local escort = require "escort"

local autonav, target_pos, target_spb, target_plt, target_name, instant_jump
local autonav_jump_delay, autonav_jump_approach, autonav_jump_brake
local autonav_pos_approach_brake, autonav_pos_approach, _autonav_abort
local autonav_spob_approach_brake, autonav_spob_approach, autonav_spob_land_approach, autonav_spob_land_brake
local autonav_plt_follow, autonav_plt_board_approach
local autonav_timer, tc_base, tc_mod, tc_max, tc_rampdown, tc_down
local last_shield, last_armour, map_npath, reset_shield, reset_dist, reset_lockon, fleet_speed, game_speed, escort_health
local path, uselanes_jump, uselanes_spob, uselanes_thr, match_fleet, follow_land_jump, brake_pos, include_escorts
local follow_pilot_fleet
local already_aboff
local escorting
local wait_msg

-- We'll need the physics constants
local PHYSICS_SPEED_DAMP = require("constants").PHYSICS_SPEED_DAMP

-- Some defaults
autonav_timer = 0
tc_mod = 0
map_npath = 0
tc_down = 0
uselanes_jump = true
uselanes_spob = false
include_escorts = true
last_shield = 0
last_armour = 0
escort_health = {}
follow_pilot_fleet = {} -- Pilots in the fleet the player is following

--[[
Common code for setting decent defaults and global variables when starting autonav.
--]]
local function autonav_setup ()
   -- Get player / game info
   local pp = player.pilot()
   instant_jump = pp:shipstat("misc_instant_jump")
   game_speed = naev.conf().game_speed

   -- Some safe defaults
   autonav     = nil
   target_pos  = nil
   target_spb  = nil
   target_plt  = nil
   map_npath   = 0
   tc_rampdown = false
   tc_down     = 0
   path        = nil
   already_aboff= false
   wait_msg    = false
   follow_pilot_fleet = {}
   local stealth = pp:flags("stealth")
   uselanes_jump = var.peek("autonav_uselanes_jump") and not stealth
   uselanes_spob = var.peek("autonav_uselanes_spob") and not stealth
   uselanes_thr = var.peek("autonav_uselanes_thr") or 2
   match_fleet = var.peek("autonav_match_fleet")
   follow_land_jump = var.peek("autonav_follow_jump")
   reset_shield = var.peek("autonav_reset_shield") * 100 -- Has to be in percent
   reset_dist = var.peek("autonav_reset_dist")
   brake_pos = var.peek("autonav_brake_pos")
   reset_lockon = var.peek("autonav_reset_lockon")
   include_escorts = var.peek("autonav_include_escorts")
   player.autonavSetPos()

   -- See if escorting
   local escorts = escort.all_mission_pilots()
   if match_fleet and (#escorts > 0) then
      escorting = escorts
   else
      escorting = nil
   end

   -- Set time compression maximum
   tc_max = var.peek("autonav_compr_speed") / pp:speedMax()
   local compr_max = var.peek("autonav_compr_max")
   tc_max = math.max( 1, math.min( tc_max, compr_max ) )

   -- Set initial time compression base
   tc_base = player.dt_default() * player.speed()
   tc_mod = math.max( tc_base, tc_mod )
   player.autonavSetSpeed( tc_mod, nil )

   -- Initialize health
   last_shield, last_armour = pp:health()
   escort_health = {}
   for k,p in ipairs(pp:followers()) do
      escort_health[ p:id() ] = { p:health() }
   end

   -- Compute slowest fleet speed
   local followers = pp:followers()
   fleet_speed = math.huge
   if #followers <= 0 then
      match_fleet = false
   else
      for k,p in pairs(followers) do
         fleet_speed = math.min( fleet_speed, p:speedMax() )
      end
   end

   -- Send message to follows to regroup
   if include_escorts then
      pp:msg( pp:followers(), "e_autonav" )
   end

   -- Start timer to begin time compressing right away
   autonav_timer = 0
end

local approach_brake = false
local function autonav_set( func )
   approach_brake = false
   autonav = func
end

local function resetSpeed ()
   tc_mod = 1
   tc_rampdown = false
   tc_down = 0
   player.autonavSetSpeed( nil, nil ) -- restore sped
end

local function shouldResetSpeed ()
   local pp = player.pilot()
   if reset_lockon and pp:lockon() > 0 then
      resetSpeed()
      return true
   end

   local will_reset = (autonav_timer > 0)

   local armour, shield = pp:health()
   local lowhealth = (shield < last_shield and shield < reset_shield) or (armour < last_armour)
   if include_escorts then
      for k,p in ipairs(pp:followers()) do
         local a, s = p:health()
         local h = escort_health[ p:id() ]
         local la, ls
         if not h then
            la, ls = 0, 0
         else
            la, ls = table.unpack(h)
         end
         lowhealth = lowhealth or ((s < ls and s < reset_shield) or (a < la))
         escort_health[ p:id() ] = {a,s}
      end
   end

   -- First check enemies in distance which should be fast
   if not will_reset and reset_dist > 0 then
      for k,p in ipairs(pp:getEnemies( reset_dist )) do
         will_reset = true
         autonav_timer = math.max( autonav_timer, 0 )
      end
   end

   -- Next, check if enemy is nearby when we are low on health
   if not will_reset and lowhealth then
      for k,p in ipairs(pp:getVisible()) do
         if pp:areEnemies(p) then
            local inrange, known = pp:inrange( p )
            if inrange and known then
               if lowhealth then
                  will_reset = true
                  autonav_timer = math.max( autonav_timer, 2 )
                  break
               end
            end
         end
      end
   end

   last_shield = shield
   last_armour = armour

   if will_reset then
      resetSpeed()
      return true
   end
   return false
end

local setting_target = false
local function set_pilot_target( plt )
   setting_target = true
   player.pilot():setTarget( plt )
   setting_target = false
end

local function get_pilot_name( plt )
   local _inrng, target_known
   if plt:exists() then
      _inrng, target_known = player.pilot():inrange( plt )
   end
   if target_known then
      return "#"..plt:colourChar()..plt:name().."#o"
   elseif target_name then
      return target_name
   else
      return _("Unknown")
   end
end

local function get_sys_name( sys )
   if sys:known() then
      return sys:name()
   end
   return _("Unknown")
end

local function get_spob_name( spb )
   return "#"..spb:colourChar()..spb:name().."#o"
end

--[[
Triggered when a mission or the likes temporarily disables autonav.
--]]
function autonav_reset( time )
   resetSpeed()
   autonav_timer = math.max( autonav_timer, time )
end

--[[
Triggers when autonav is successfully terminated or cleaning up.
--]]
function autonav_end ()
   resetSpeed()
   player.autonavEnd()
end

--[[
Autonav to a system, destination is in the player's nav
--]]
local function _autonav_system (do_uselanes)
   local dest
   dest, map_npath = player.autonavDest()
   player.msg("#o"..fmt.f(_("Autonav: travelling to {sys}."),{sys=get_sys_name(dest)}).."#0")

   local pp = player.pilot()
   local jmp = pp:navJump()
   local pos = jmp:pos()
   local d = jmp:jumpDist( pp )
   target_pos = pos + (pp:pos()-pos):normalize( math.max(0.75*d, d-50) )

   if do_uselanes then
      lanes.clearCache( pp )
      path = lanes.getRouteP( pp, target_pos, nil, uselanes_thr )
   else
      path = {target_pos}
   end

   if pilot.canHyperspace(pp) then
      autonav_set( autonav_jump_brake )
   else
      autonav_set( autonav_jump_approach )
   end
end

function autonav_system ()
   autonav_setup()
   _autonav_system (uselanes_jump)
end

--[[
Autonav to a spob, potentially trying to land
--]]
local function _autonav_spob(spb, tryland, do_uselanes)
   target_spb = spb
   local pp = player.pilot()
   local pos = spb:pos()
   target_pos = pos + (pp:pos()-pos):normalize( 0.6*spb:radius() )

   if do_uselanes then
      lanes.clearCache( pp )
      path = lanes.getRouteP( pp, target_pos, nil, uselanes_thr )
   else
      path = {target_pos}
   end

   local spobstr = get_spob_name( spb )
   if tryland then
      player.msg("#o"..fmt.f(_("Autonav: landing on {spob}."),{spob=spobstr}).."#0")
      autonav_set( autonav_spob_land_approach )
   else
      player.msg("#o"..fmt.f(_("Autonav: approaching {spob}."),{spob=spobstr}).."#0")
      autonav_set( autonav_spob_approach )
   end

end

function autonav_spob( spb, tryland )
   autonav_setup()
   _autonav_spob( spb, tryland, uselanes_spob )
end

local function pilot_fleet ( plt )
   local flt

   -- Check if leader
   local l = plt:leader()
   if l ~= nil then
      -- We'll initialize to leader and get followers, which should include the current pilot
      flt = { l }
      for k,v in ipairs( l:followers() ) do
         table.insert( flt, v )
      end
   else
      -- Assume pilot is going to be the leader
      flt = { plt }
   end

   -- Add pilot's followers
   for k,v in ipairs( plt:followers() ) do
      table.insert( flt, v )
   end

   -- Sort
   local ppos = player.pos()
   table.sort( flt, function( a, b )
      return a:pos():dist2( ppos ) < b:pos():dist2( ppos )
   end )

   return flt
end

--[[
Autonav to follow a target pilot
--]]
function autonav_pilot( plt )
   autonav_setup()
   target_plt = plt
   local pltstr
   local _inrng, known = player.pilot():inrange( plt )
   if known then
      pltstr = "#"..plt:colourChar()..plt:name().."#o"
      target_name = pltstr
   else
      pltstr = _("Unknown")
      target_name = nil
   end

   player.msg("#o"..fmt.f(_("Autonav: following {plt}."),{plt=pltstr}).."#0")
   autonav_set( autonav_plt_follow )

   -- Get the fleet, we'll try to follow someone in fleet when lost
   follow_pilot_fleet = pilot_fleet( plt )
end

--[[
Autonav to board a pilot
--]]
function autonav_board( plt )
   autonav_setup()
   target_plt = plt
   local pltstr
   local _inrng, known = player.pilot():inrange( plt )
   if known then
      pltstr = "#"..plt:colourChar()..plt:name().."#o"
      target_name = pltstr
   else
      pltstr = _("Unknown")
      target_name = nil
   end
   player.msg("#o"..fmt.f(_("Autonav: boarding {plt}."),{plt=pltstr}).."#0")
   autonav_set( autonav_plt_board_approach )
end

--[[
Autonav to a position specified by the player
--]]
function autonav_pos( pos )
   autonav_setup()
   player.msg("#o".._("Autonav: heading to target position.").."#0")
   autonav_set( autonav_pos_approach )
   player.autonavSetPos( pos )
   target_pos = pos
   path = {target_pos}
end

--[[
tint is the integral of the time in per time units.

 tc_mod
    ^
    |
    |\
    | \
    |  \___
    |
    +------> time
    0   3

We decompose integral in a rectangle (3*1) and a triangle (3*(tc_mod-1.))/2.
This is the "elapsed time" when linearly decreasing the tc_mod. Which we can
use to calculate the actual "game time" that'll pass when decreasing the
tc_mod to 1 during 3 seconds. This can be used then to compare when we want to
start decrementing.
--]]
local function autonav_rampdown( count_brake )
   -- Compute accurate distance to the end
   local d = 0
   local pos = player.pos()
   for k,v in ipairs(path) do
      d = d+pos:dist( v )
      pos = v
   end

   local pp = player.pilot()
   -- Compute velocity in "real pixels / second"
   local vel = math.min( pp:speedMax(), pp:vel():mod() ) * game_speed
   local acc = (tc_mod - tc_base) / PHYSICS_SPEED_DAMP -- Acceleration
   local dtravel = vel * 3 * (tc_mod - 0.5 * PHYSICS_SPEED_DAMP * acc)
   local bdist, btime = ai.minbrakedist()
   if not count_brake then
      -- Autonav stops at minbrakedist, so this synchronizes it
      d = d - bdist
   else
      d = d + btime * vel * tc_base
   end
   if dtravel > d then
      tc_rampdown = true
      tc_down     = acc
   end
end

local function turnoff_afterburner()
   local pp = player.pilot()
   for _i,n in ipairs(pp:actives()) do
      -- All movement outfits will break autonav, however, many movement
      -- outfits are instant and won't be caught by this polling scheme.
      if n.outfit:tags().movement and n.state=="on" then
         if already_aboff then
            return _autonav_abort(_("manual commands at approach"))
         else
            if not pp:outfitToggle( n.slot, false ) then
               -- Failed to disable
               return _autonav_abort(_("manual commands at approach"))
            end
         end
      end
   end
   already_aboff = true
end

--[[
   For approaching a static target.
--]]
local function autonav_approach( pos, count_brakedist )
   local pp = player.pilot()
   local brakedist = ai.minbrakedist()
   local d = pos:dist( pp:pos() )
   local off = ai.iface( pos )
   if not match_fleet then
      if off < math.rad(10) then
         ai.accel(1)
      end
   else
      local dir = vec2.newP( 1, pp:dir() )
      local vel = pp:vel()
      local dot = (vel+dir*0.001):normalize():dot( dir )
      local mod = vel:mod()
      if approach_brake then
         if mod > 0.8*fleet_speed and (mod*dot > 0.7*fleet_speed or dot < 0.8) then
            ai.brake(true)
         else
            approach_brake = false
         end
      else
         if off < math.rad(10) and (mod*dot < 0.95*fleet_speed) then
            ai.accel(1)
         elseif mod > fleet_speed and ((dot < 0.86) or pp:speed() > 0.98*fleet_speed) then -- More than 30 degree error
            ai.brake(true)
            approach_brake = true
         end
      end
   end

   -- Distance left to start breaking
   local dist = d - brakedist
   local retd
   if count_brakedist then
      retd = dist
   else
      retd = d
   end

   if dist < 0 then
      turnoff_afterburner()
      ai.accel(0)
      return true, retd
   end
   return false, retd
end

-- For approaching a target with velocity, and staying at a radius distance
local function autonav_approach_vel( pos, vel, radius )
   local pp = player.pilot()
   local turn = pp:turn() -- TODO probably change the code

   local rvel = vel - pp:vel()
   local timeFactor = math.pi/turn + rvel:mod() / pp:accel() / 2

   local Kp = 10
   local Kd = math.max( 5, 10*timeFactor )

   local angle = math.pi + vel:angle()

   local point = pos + vec2.newP( radius, angle )
   local dir = (point-pp:pos())*Kp + rvel*Kd

   local off = ai.face( dir:angle() )
   if math.abs(off) < math.rad(10) and dir:mod() > 300 then
      ai.accel(1)
   end

   return pos:dist( pp:pos() )
end

-- Jumped into a system and delaying until velocity is somewhat normal
function autonav_jump_delay ()
   -- Ignore autonav until speed is acceptable
   local pp = player.pilot()
   -- hades torch: speed*1.65
   -- accel * 1.0/3 goes as speed bonus
   if pp:vel():mod() > 1.65 * pp:speedMax() + pp:accel()/PHYSICS_SPEED_DAMP then
      return
   end

   -- Determine how to do lanes
   if uselanes_jump then
      path = lanes.getRouteP( pp, target_pos, nil, uselanes_thr )
   else
      path = {target_pos}
   end
   autonav_set( autonav_jump_approach )
end

local function recompute_jump_pos ()
   local pp = player.pilot()
   local jmp = pp:navJump()
   local pos = jmp:pos()
   local d = jmp:jumpDist( pp )
   target_pos = pos + (pp:pos()-pos):normalize( math.max(0.8*d, d-30) )
   path = {target_pos} -- Have to update path also
end

-- Approaching a jump point, target position is stored in target_pos
function autonav_jump_approach ()
   local pp = player.pilot()
   local jmp = pp:navJump()
   if not jmp then
      return _autonav_abort()
   end
   local ret = autonav_approach( path[1], true )
   if ret then
      if #path > 1 then
         table.remove( path, 1 )
      else
         -- If the original position does not work, try to get closer before braking
         if jmp:pos():dist(path[1]) > jmp:jumpDist( pp ) then
            recompute_jump_pos()
         else
            autonav_set( autonav_jump_brake )
         end
      end
   elseif not tc_rampdown and map_npath<=1 then
      autonav_rampdown( true )
   end
end

-- More approaching a jump point and turning in the direction of the jump out.
local function autonav_instant_jump_final_approach ()
   -- This function assumes there is the jump point in the front of player.
   local pp = player.pilot()

   local jmp = pp:navJump()
   local jmp_pos = jmp:pos()
   local jmp_r_pos = jmp_pos - pp:pos()

   -- The reference angle is the running direction of player.
   local pp_vel = pp:vel()
   local ref_vec = vec2.clone(pp_vel):normalize() -- need to copy

   local x = vec2.dot( ref_vec, jmp_r_pos )

   if x < 0 then
      -- player passed by the jump point.
      return true
   end

   -- Estimate the turning time and the running distance.
   local jmpout_dir = -jmp:angle()
   local diff_dir = (((pp:dir() - jmpout_dir) / (2.0 * math.pi) + 0.5) % 1.0 - 0.5) * 2.0 * math.pi
   local turn_time = math.abs(diff_dir) / pp:turn()
   local turn_dist = turn_time * vec2.dot( ref_vec, pp_vel )

   -- The distance to the position where player can jump out.
   local jmp_dist = jmp_r_pos:dist() - jmp:jumpDist( pp )

   if jmp_dist <= turn_dist then
      -- Turning in the direction of the jump out.
      ai.accel(0)
      ai.face( jmpout_dir )
   else
      -- Approaching the jump point.
      ai.face( jmp_pos )
      ai.accel(1)
   end
   return false
end

local function escorts_left_jump ()
   if not escorting then return false end
   for k,p in ipairs(escorting) do
      if p:exists() and not p:flags("jumpingout") then
         return true
      end
   end
   return false
end

-- Breaking at a jump point, target position is stored in target_pos
function autonav_jump_brake ()
   local ret
   -- With instant jumping we can just focus on getting in range
   if instant_jump and not escorts_left_jump() then
      ret = autonav_instant_jump_final_approach()
   else
      ret = ai.brake()
   end

   if ai.canHyperspace() then
      if escorts_left_jump() then
         if not wait_msg then
            player.msg("#0".._("Autonav: waiting for escorts to jump first.").."#0")
            wait_msg = true
         end
         return -- wait
      end

      ai.hyperspace()
      local pp = player.pilot()
      pp:msg( pp:followers(), "hyperspace", pp:navJump() )
   elseif ret then
      -- Recompute the location for a better position
      recompute_jump_pos()
      autonav_set( autonav_jump_approach )
   elseif not tc_rampdown and map_npath<=1 then
      autonav_rampdown( true )
   end
end

local function _autonav_pos_approach_brake()
   if ai.brake() then
      return true
   elseif not tc_rampdown then
      tc_rampdown = true
      tc_down     = (tc_mod - tc_base) / 3
   end
end

-- Brakes at a position
function autonav_pos_approach_brake ()
   if _autonav_pos_approach_brake() then
      player.msg("#o".._("Autonav: arrived at position.").."#0")
      return autonav_end()
   end
end

local function autonav_pos_approach_brake_silent ()
   if _autonav_pos_approach_brake() then
      return autonav_end()
   end
end

function _autonav_abort( reason )
   autonav_abort( reason, brake_pos )
end

--[[
Autonav was forcibly aborted for a reason or other.
--]]
function autonav_abort( reason, dobrake )
   -- Horrible hack so setting the target doesn't break autonav
   if setting_target then return end

   if reason then
      player.msg("#r"..fmt.f(_("Autonav: aborted due to '{reason}'!"),{reason=reason}).."#0")
   else
      player.msg("#r".._("Autonav: aborted!").."#0")
   end
   if dobrake then
      autonav_reset(0)
      autonav_set( autonav_pos_approach_brake_silent )
   else
      autonav_end()
   end
end

-- Approaching a position specified by target_pos
function autonav_pos_approach ()
   local ret = autonav_approach( target_pos, brake_pos )
   if ret then
      if brake_pos then
         autonav_set( autonav_pos_approach_brake )
      else
         player.msg("#o".._("Autonav: arrived at position.").."#0")
         return autonav_end()
      end
   elseif not tc_rampdown then
      autonav_rampdown( brake_pos )
   end
end

-- Brakes at a position
function autonav_spob_approach_brake ()
   if ai.brake() then
      player.msg("#o"..fmt.f(_("Autonav: arrived at {spob}."),{spob=get_spob_name(target_spb)}).."#0")
      return autonav_end()
   end
   if not tc_rampdown then
      tc_rampdown = true
      tc_down     = (tc_mod - tc_base) / 3
   end
end

-- Approaching a spob, not interested in landing
function autonav_spob_approach ()
   local ret = autonav_approach( path[1], brake_pos )
   if ret then
      if #path > 1 then
         table.remove( path, 1 )
      else
         if brake_pos then
            autonav_set( autonav_spob_approach_brake )
         else
            player.msg("#o"..fmt.f(_("Autonav: arrived at {spob}."),{spob=get_spob_name(target_spb)}).."#0")
            return autonav_end()
         end
      end
   elseif not tc_rampdown then
      autonav_rampdown( brake_pos )
   end
end

-- Approaching a spob to try to land
function autonav_spob_land_approach ()
   local ret = autonav_approach( path[1], true )
   if ret then
      if #path > 1 then
         table.remove( path, 1 )
      else
         autonav_set( autonav_spob_land_brake )
      end
   elseif not tc_rampdown then
      autonav_rampdown( true )
   end
end

local function escorts_left_land ()
   if not escorting then return false end
   for k,p in ipairs(escorting) do
      if p:exists() and not p:flags("landing") then
         return true
      end
   end
   return false
end

-- Going for the landing approach
function autonav_spob_land_brake ()
   local ret = ai.brake()

   -- See if we have to wait for escorts
   if escorts_left_land() then
      if not wait_msg then
         player.msg("#0".._("Autonav: waiting for escorts to land first.").."#0")
         wait_msg = true
      end
      return
   end

   if player.tryLand(false)=="impossible" then
      return _autonav_abort(_("cannot land"))
   end

   if ret then
      -- Reset to good position
      local pp = player.pilot()
      local pos = target_spb:pos()
      target_pos = pos + (pp:pos()-pos):normalize( 0.6*target_spb:radius() )
      path = {target_pos} -- Have to update path also
      autonav_set( autonav_spob_land_approach )
   elseif not tc_rampdown then
      tc_rampdown = true
      tc_down     = (tc_mod - tc_base) / 3
   end
end

-- Following a target pilot
function autonav_plt_follow ()
   local plt = target_plt
   local target_known = false
   local inrng = false
   local pp = player.pilot()

   if plt:exists() then
      if plt:flags("jumpingout") then
         local jmp = plt:navJump()
         player.msg("#o"..fmt.f(_("Autonav: following target {plt} has jumped to {sys}."),{plt=get_pilot_name(plt),sys=get_sys_name(jmp:dest())}).."#0")

         if follow_land_jump then
            local fuel, consumption = player.fuel()
            if jmp:known() and not jmp:exitonly() and fuel >=consumption then

               -- See if there's someone else in the fleet we can follow now
               for k,p in ipairs( pilot_fleet( plt ) ) do
                  if p:exists() and not p:flags("jumpingout") then
                     set_pilot_target( p )
                     autonav_pilot( p )
                     return
                  end
               end

               -- Try to follow through jump
               pp:navJumpSet( jmp )
               _autonav_system(false)
               autonav_reset(0)
            else
               local why=nil
               if fuel < consumption then
                  if fuel == 0 then
                     why=_("no fuel")
                  else
                     why=_("not enough fuel")
                  end
               end
               player.msg("#o"..fmt.f(_("Autonav: Could not follow target {plt} by jumping."),{plt=get_pilot_name(plt)}).."#0")
               return _autonav_abort(why)
            end
         elseif brake_pos then
            pp:navJumpSet( jmp )
            autonav_pos(plt:navJump():pos())
            autonav_reset(0)
            --autonav_rampdown( false )
         else
            autonav_end()
         end
         return
      elseif plt:flags("landing") then
         player.msg("#o"..fmt.f(_("Autonav: following target {plt} has landed on {spb}."),{plt=get_pilot_name(plt),spb=get_spob_name(plt:navSpob())}).."#0")

         if follow_land_jump or brake_pos then

            -- See if there's someone else in the fleet we can follow now
            for k,p in ipairs( pilot_fleet( plt ) ) do
               if p:exists() and not p:flags("landing") then
                  set_pilot_target( p )
                  autonav_pilot( p )
                  return
               end
            end

            -- Try to follow to land
            pp:navSpobSet( plt:navSpob() )
            _autonav_spob( plt:navSpob(), follow_land_jump, false) -- do it without following lanes
            if follow_land_jump then
               autonav_reset(0)
            else
               autonav_rampdown( true )
            end
         else
            autonav_end()
         end
         return
      end
      inrng, target_known = pp:inrange( plt )
   end

   if not inrng then -- If doesn't exist defaults to false
      local pltstr = get_pilot_name( plt )
      player.msg("#r"..fmt.f(_("Autonav: following target {plt} has been lost."),{plt=pltstr}).."#0")

      -- See if there's someone else in the original fleet we can follow now
      for k,p in ipairs( follow_pilot_fleet ) do
         if p:exists() and not p:flags("landing") and pp:inrange(p) then
            set_pilot_target( p )
            autonav_pilot( p )
            return
         end
      end

      ai.accel(0)
      return autonav_end()
   elseif not target_name and target_known then
      target_name = "#"..plt:colourChar()..plt:name().."#o"
   end

   local canboard = plt:flags("disabled") or plt:flags("boardable")
   local radius
   if canboard then
      radius = 0
   elseif pp:flags("stealth") then
      -- Follow with the same range that stops stealth (see pilot_ewStealthGetNearby)
      local stealthrange = 1.5 * pp:stealthRange() * plt:shipstat( "ew_detect", true )
      radius = pp:radius()+plt:radius()+stealthrange
   else
      radius = math.max( 100, 1.5*(pp:radius()+plt:radius()) )
   end
   local pos = plt:pos()
   autonav_approach_vel( pos, plt:vel(), radius )
   -- Only ramps down if we can board
   if canboard and not tc_rampdown then
      path = {pos}
      autonav_rampdown(true)
   end
end

-- Getting close to board a pilot
function autonav_plt_board_approach ()
   local plt = target_plt
   local target_known = false
   local inrng = false
   if plt:exists() then
      inrng, target_known = player.pilot():inrange( plt )
   end

   if not inrng then
      player.msg("#r"..fmt.f(_("Autonav: boarding target {plt} has been lost."),{plt=get_pilot_name(plt)}).."#0")
      ai.accel(0)
      return autonav_end()
   elseif not target_name and target_known then
      target_name = "#"..plt:colourChar()..plt:name().."#o"
   end

   local pos = plt:pos()
   autonav_approach_vel( pos, plt:vel(), 0 )
   if not tc_rampdown then
      path = {pos}
      autonav_rampdown(true)
   end

   -- Finally try to board
   local brd = player.tryBoard(false)
   if brd=="ok" then
      autonav_end()
   elseif brd~="retry" then
      _autonav_abort(_("cannot board"))
   end
end

-- Run quite often when the player tries to think. dt is game time
function autonav_think( dt )
   if autonav_timer > 0 then
      autonav_timer = autonav_timer - dt
   end

   if autonav then
      autonav()
   end
end

-- Run with the physics backend
function autonav_update( realdt )
   -- If we reset we skip the iteration
   if shouldResetSpeed() then
      return
   end
   local dt_default = player.dt_default()
   if tc_rampdown then
      if tc_mod > tc_base then
         tc_mod = tc_mod - tc_down * realdt
         tc_mod = math.max( tc_mod, tc_base )
         player.autonavSetSpeed( tc_mod, tc_mod / dt_default )
      end
      return
   end

   if tc_mod == tc_max then
      return
   end
   -- 5 seconds to reach max speed
   tc_mod = tc_mod + 0.2 * realdt * (tc_max - tc_base )
   tc_mod = math.min( tc_mod, tc_max )
   player.autonavSetSpeed( tc_mod, tc_mod / dt_default )
end

function autonav_enter ()
   local dest
   wait_msg = false
   dest, map_npath = player.autonavDest()
   if autonav==autonav_jump_approach or autonav==autonav_jump_brake then
      if not dest then
         dest = system.cur()
      end
      local pp = player.pilot()
      local jmp = pp:navJump()
      local sysstr = get_sys_name( dest )

      -- Made it to target
      if jmp==nil then
         player.msg("#o"..fmt.f(_("Autonav arrived at the {sys} system."),{sys=sysstr}).."#0")
         return autonav_end()
      end

      -- Must have fuel to continue
      local fuel, consumption = player.fuel()
      if fuel < consumption then
         _autonav_abort(_("not enough fuel to continue"))
         return false
      end

      -- Keep on going
      player.msg("#o"..fmt.f(n_(
         "Autonav continuing until {sys} ({n} jump left).",
         "Autonav continuing until {sys} ({n} jumps left).",
         map_npath),{sys=sysstr,n=map_npath}).."#0")

      local pos = jmp:pos()
      local d = jmp:jumpDist( pp )
      target_pos = pos + (pp:pos()-pos):normalize( math.max(0.8*d, d-30) )
      if uselanes_jump then
         lanes.clearCache( pp )
      end
      autonav_set( autonav_jump_delay )
   end
end
