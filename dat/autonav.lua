local fmt = require "format"
local lanes = require "ai.core.misc.lanes"

local autonav, target_pos, target_spb, target_plt, instant_jump
local autonav_jump_delay, autonav_jump_approach, autonav_jump_brake
local autonav_pos_approach
local autonav_spob_approach, autonav_spob_land_approach, autonav_spob_land_brake
local autonav_plt_follow, autonav_plt_board_approach
local autonav_timer, tc_base, tc_mod, tc_max, tc_rampdown, tc_down
local last_shield, last_armour, map_npath, reset_shield, reset_dist, reset_lockon
local path, uselanes_jump, uselanes_spob

-- Some defaults
autonav_timer = 0
tc_mod = 0
map_npath = 0
tc_down = 0
uselanes_jump = true
uselanes_spob = false

--[[
Common code for setting decent defaults and global variables when starting autonav.
--]]
local function autonav_setup ()
   -- Get player / game info
   local pp = player.pilot()
   instant_jump = pp:shipstat("misc_instant_jump")

   -- Some safe defaults
   autonav     = nil
   target_pos  = nil
   target_spb  = nil
   target_plt  = nil
   map_npath   = 0
   tc_rampdown = false
   tc_down     = 0
   path        = nil
   local stealth = pp:flags("stealth")
   uselanes_jump = var.peek("autonav_uselanes_jump") and not stealth
   uselanes_spob = var.peek("autonav_uselanes_spob") and not stealth
   reset_shield = var.peek("autonav_reset_shield")
   reset_dist = var.peek("autonav_reset_dist")
   reset_lockon = true
   player.autonavSetPos()

   -- Set time compression maximum
   tc_max = var.peek("autonav_compr_speed") / pp:speedMax()
   local compr_max = var.peek("autonav_compr_max")
   tc_max = math.max( 1, math.min( tc_max, compr_max ) )

   -- Set initial time compression base
   tc_base = player.dt_default() * player.speed()
   tc_mod = math.max( tc_base, tc_mod )
   player.setSpeed( tc_mod, nil, true )

   -- Initialize health
   last_shield, last_armour = pp:health()

   -- Start timer to begin time compressing right away
   autonav_timer = 0
end

local function resetSpeed ()
   tc_mod = 1
   tc_rampdown = false
   tc_down = 0
   player.setSpeed( nil, nil, true ) -- restore sped
end

local function shouldResetSpeed ()
   local pp = player.pilot()
   if reset_lockon and pp:lockon() > 0 then
      resetSpeed()
      return true
   end

   local will_reset = (autonav_timer > 0)

   local armour, shield = pp:health()
   local lowhealth = (shield < last_shield and reset_shield) or (armour < last_armour)

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
function autonav_system ()
   autonav_setup()
   local dest
   dest, map_npath = player.autonavDest()
   local sysstr
   if dest:known() then
      sysstr = dest:name()
   else
      sysstr = _("Unknown")
   end
   player.msg("#o"..fmt.f(_("Autonav: travelling to {sys}."),{sys=sysstr}).."#0")

   local pp = player.pilot()
   local jmp = pp:navJump()
   local pos = jmp:pos()
   local d = jmp:jumpDist( pp )
   target_pos = pos + (pp:pos()-pos):normalize( math.max(0.8*d, d-30) )

   if uselanes_jump then
      lanes.clearCache( pp )
      path = lanes.getRouteP( pp, target_pos )
   else
      path = {target_pos}
   end

   if pilot.canHyperspace(pp) then
      autonav = autonav_jump_brake
   else
      autonav = autonav_jump_approach
   end
end

--[[
Autonav to a spob, potentially trying to land
--]]
function autonav_spob( spb, tryland )
   autonav_setup()
   target_spb = spb
   local pp = player.pilot()
   local pos = spb:pos()
   target_pos = pos + (pp:pos()-pos):normalize( 0.6*spb:radius() )

   if uselanes_spob then
      lanes.clearCache( pp )
      path = lanes.getRouteP( pp, target_pos )
   else
      path = {target_pos}
   end

   local spobstr = "#"..spb:colourChar()..spb:name().."#o"
   if tryland then
      player.msg("#o"..fmt.f(_("Autonav: landing on {spob}."),{spob=spobstr}).."#0")
      autonav = autonav_spob_land_approach
   else
      player.msg("#o"..fmt.f(_("Autonav: approaching {spob}."),{spob=spobstr}).."#0")
      autonav = autonav_spob_approach
   end

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
   else
      pltstr = _("Unknown")
   end

   player.msg("#o"..fmt.f(_("Autonav: following {plt}."),{plt=pltstr}).."#0")
   autonav = autonav_plt_follow
end

--[[
Autonav to board a pilot
--]]
function autonav_board( plt )
   autonav_setup()
   target_plt = plt
   local pltstr = "#"..plt:colourChar()..plt:name().."#o"
   player.msg("#o"..fmt.f(_("Autonav: boarding {plt}."),{plt=pltstr}).."#0")
   autonav = autonav_plt_board_approach
end

--[[
Autonav to a position specified by the plyaer
--]]
function autonav_pos( pos )
   autonav_setup()
   player.msg("#o".._("Autonav: heading to target position.").."#0")
   autonav = autonav_pos_approach
   player.autonavSetPos( pos )
   target_pos = pos
end

--[[
Autonav was forcibly aborted for a reason or other.
--]]
function autonav_abort( reason )
   if reason then
      player.msg("#r"..fmt.f(_("Autonav: aborted due to '{reason}'!"),{reason=reason}).."#0")
   else
      player.msg("#r".._("Autonav: aborted!").."#0")
   end
   autonav_end()
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
local function autonav_rampdown( d )
   local pp = player.pilot()
   local speed = pp:speed()

   local vel = math.min( 1.5*speed, pp:vel():mod() )
   local t   = d / vel * (1 - 0.075 * tc_base)
   local tint= 3 + 0.5*3*(tc_mod-tc_base)
   if t < tint then
      tc_rampdown = true
      tc_down     = (tc_mod-tc_base) / 3
   end
end

-- For getting close to a static target
local function autonav_approach( pos, count_target )
   local pp = player.pilot()
   local dist = ai.minbrakedist()
   local d = pos:dist( pp:pos() )
   local off
   if d < dist*3 then
      off = ai.iface( pos )
   else
      off = ai.face( pos )
   end
   if off < math.rad(10) then
      ai.accel(1)
   end

   dist = d - dist
   local retd
   if count_target then
      retd = dist
   else
      retd = d
   end

   if dist < 0 then
      ai.accel(0)
      return true, retd
   end
   return false, retd
end

-- For approaching a target with velocity, and staying at a radius distance
local function autonav_approach_vel( pos, vel, radius )
   local pp = player.pilot()
   local turn = math.rad(pp:turn()) -- TODO probably change the code

   local rvel = vel - pp:vel()
   local timeFactor = math.pi/turn + rvel:mod() / pp:thrust() / 2

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
   if pp:vel():mod() > 1.5 * pp:speedMax() then
      return
   end

   -- Determine how to do lanes
   if uselanes_jump then
      path = lanes.getRouteP( pp, target_pos )
   else
      path = {target_pos}
   end
   autonav = autonav_jump_approach
end

-- Approaching a jump point, target position is stored in target_pos
function autonav_jump_approach ()
   local pp = player.pilot()
   local jmp = pp:navJump()
   if not jmp then
      return autonav_abort()
   end
   local ret, d = autonav_approach( path[1], false )
   if ret then
      if #path > 1 then
         table.remove( path, 1 )
      else
         autonav = autonav_jump_brake
      end
   elseif not tc_rampdown and map_npath<=1 and #path==1 then
      autonav_rampdown( d )
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
   local ref_vec = vec2.copy(pp_vel):normalize() -- need to copy

   local x = vec2.dot( ref_vec, jmp_r_pos )

   if x < 0 then
      -- player passed by the jump point.
      return true
   end

   -- Estimate the turning time and the running distance.
   local jmpout_dir = -jmp:angle()
   local diff_dir = ((pp:dir() - jmpout_dir) / (2.0 * math.pi) + 0.5) % 1.0 - 0.5
   local turn_time = math.abs(diff_dir) * 360 / pp:turn()
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

-- Breaking at a jump point, target position is stored in target_pos
function autonav_jump_brake ()
   local ret
   -- With instant jumping we can just focus on getting in range
   if instant_jump then
      ret = autonav_instant_jump_final_approach()
   else
      ret = ai.brake()
   end

   if ai.canHyperspace() then
      ai.hyperspace()
   elseif ret then
      -- Recompute the location for a better position
      local pp = player.pilot()
      local jmp = pp:navJump()
      local pos = jmp:pos()
      local d = jmp:jumpDist( pp )
      target_pos = pos + (pp:pos()-pos):normalize( math.max(0.8*d, d-30) )
      autonav = autonav_jump_approach
   end

   if not tc_rampdown and map_npath<=1 then
      tc_rampdown = true
      tc_down     = (tc_mod-tc_base) / 3
   end
end

-- Approaching a position specified by target_pos
function autonav_pos_approach ()
   local ret, d = autonav_approach( target_pos, true )
   if ret then
      player.msg("#o".._("Autonav: arrived at position.").."#0")
      return autonav_end()
   elseif not tc_rampdown then
      autonav_rampdown( d )
   end
end

-- Approaching a spob, not interested in landing
function autonav_spob_approach ()
   local ret = autonav_approach( path[1], true )
   if ret then
      if #path > 1 then
         table.remove( path, 1 )
      else
         local spobstr = "#"..target_spb:colourChar()..target_spb:name().."#o"
         player.msg("#o"..fmt.f(_("Autonav: arrived at {spob}."),{spob=spobstr}).."#0")
         return autonav_end()
      end
   elseif not tc_rampdown then
      -- Use distance to end
      autonav_rampdown( player.pos():dist(target_pos) )
   end
end

-- Approaching a spob to try to land
function autonav_spob_land_approach ()
   local ret = autonav_approach( path[1], true )
   if ret then
      if #path > 1 then
         table.remove( path, 1 )
      else
         autonav = autonav_spob_land_brake
      end
   elseif not tc_rampdown then
      -- Use distance to end
      autonav_rampdown( player.pos():dist(target_pos) )
   end
end

-- Going for the landing approach
function autonav_spob_land_brake ()
   local ret = ai.brake()
   if player.tryLand(false)=="impossible" then
      return autonav_abort()
   elseif ret then
      -- Reset to good position
      local pp = player.pilot()
      local pos = target_spb:pos()
      target_pos = pos + (pp:pos()-pos):normalize( 0.6*target_spb:radius() )
      autonav = autonav_spob_land_approach
   end

   if not tc_rampdown then
      tc_rampdown = true
      tc_down     = (tc_mod-tc_base) / 3
   end
end

-- Following a target pilot
function autonav_plt_follow ()
   local plt = target_plt
   local target_known = false
   local inrng = false
   if plt:exists() then
      inrng, target_known = player.pilot():inrange( plt )
   end

   if not inrng then
      local pltstr
      if target_known then
         pltstr = "#"..plt:colourChar()..plt:name().."#o"
      else
         pltstr = _("Unknown")
      end
      player.msg("#r"..fmt.f(_("Autonav: following target {plt} has been lost."),{plt=pltstr}).."#0")
      ai.accel(0)
      return autonav_end()
   end

   local pp = player.pilot()
   local canboard = plt:flags("disabled") or plt:flags("boardable")
   local radius = math.max( 100, 1.5*(pp:radius()+plt:radius()) )
   if canboard then
      radius = 0
   end
   local d = autonav_approach_vel( plt:pos(), plt:vel(), radius )
   if canboard and not tc_rampdown then
      autonav_rampdown( d )
   end
end

-- Geting close to board a pilot
function autonav_plt_board_approach ()
   local plt = target_plt
   local target_known = false
   local inrng = false
   if plt:exists() then
      inrng, target_known = player.pilot():inrange( plt )
   end

   if not inrng then
      local pltstr
      if target_known then
         pltstr = "#"..plt:colourChar()..plt:name().."#o"
      else
         pltstr = _("Unknown")
      end
      player.msg("#r"..fmt.f(_("Autonav: boarding target {plt} has been lost."),{plt=pltstr}).."#0")
      ai.accel(0)
      return autonav_end()
   end

   local d = autonav_approach_vel( plt:pos(), plt:vel(), 0 )
   if not tc_rampdown then
      autonav_rampdown( d )
   end

   -- Finally try to board
   local brd = player.tryBoard(false)
   if brd=="ok" then
      autonav_end()
   elseif brd~="retry" then
      autonav_abort()
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

-- Run with the physics backend where realdt is user time
function autonav_update( realdt )
   -- If we reset we skip the iteration
   if shouldResetSpeed() then
      return
   end

   local dt_default = player.dt_default()
   if tc_rampdown then
      if tc_mod ~= tc_base then
         tc_mod = math.max( tc_base, tc_mod - tc_down*realdt )
         player.setSpeed( tc_mod, tc_mod / dt_default, true )
      end
      return
   end

   if tc_mod == tc_max then
      return
   end
   tc_mod = tc_mod + 0.2 * realdt * (tc_max - tc_base )
   tc_mod = math.min( tc_mod, tc_max )
   player.setSpeed( tc_mod, tc_mod / dt_default, true )
end

function autonav_enter ()
   local dest
   dest, map_npath = player.autonavDest()
   if autonav==autonav_jump_approach or autonav==autonav_jump_brake then
      if not dest then
         dest = system.cur()
      end
      local pp = player.pilot()
      local jmp = pp:navJump()
      local sysstr
      if dest:known() then
         sysstr = dest:name()
      else
         sysstr = _("Unknown")
      end

      -- Made it to target
      if jmp==nil then
         player.msg("#o"..fmt.f(_("Autonav arrived at the {sys} system."),{sys=sysstr}).."#0")
         return autonav_end()
      end

      -- Must have fuel to continue
      local fuel, consumption = player.fuel()
      if fuel < consumption then
         autonav_abort(_("Not enough fuel for autonav to continue"))
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
      autonav = autonav_jump_delay
   end
end
