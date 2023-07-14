local fmt = require "format"

local autonav, target_pos, target_spb, target_plt, instant_jump, autonav_tryland
local autonav_jump_approach, autonav_jump_brake
local autonav_pos_approach
local autonav_spob_approach, autonav_spob_land_approach, autonav_spob_land_brake
local autonav_plt_follow, autonav_plt_board_approach
local autonav_timer, tc_base, tc_mod, tc_max, tc_rampdown, tc_down
local conf, last_shield, last_armour, map_npath

-- Some defaults
tc_mod = 0
map_npath = 0
tc_down = 0

local function autonav_setup ()
   -- Get player / game info
   local pp = player.pilot()
   instant_jump = pp:shipstat("misc_instant_jump")
   conf = naev.conf()
   local stats = pp:stats()

   -- Some safe defaults
   autonav = nil
   target_pos = nil
   target_spb = nil
   target_plt = nil
   map_npath = 0
   autonav_tryland = false
   tc_rampdown = false
   tc_down     = 0
   player.autonavSetPos()

   -- Set time compression maximum
   tc_max = conf.compression_velocity / stats.speed_max
   if conf.compression_mult >= 1 then
      tc_max = math.min( tc_max, conf.compression_mult )
   end
   tc_max = math.max( 1, tc_max )

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
   if pp:lockon() > 0 then
      resetSpeed()
      return true
   end

   local reset_dist = conf.autonav_reset_dist
   local reset_shield = conf.autonav_reset_shield
   local will_reset = false

   local armour, shield = pp:health()
   local lowhealth = (shield < last_shield and reset_shield) or (armour < last_armour)

   -- First check enemies in distance which should be fast
   if reset_dist > 0 then
      for k,p in ipairs(pp:getEnemies( reset_dist )) do
         will_reset = true
         autonav_timer = math.max( autonav_timer, 0 )
      end
   end

   -- Next, check if enemy is nearby when we are low on health
   if not will_reset and lowhealth then
      for k,p in ipairs(pp:getVisible()) do
         if pp:areEnemies(p) then
            local inrange, fuzzy = pp:inrange( p )
            if inrange and not fuzzy then
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

   if will_reset or autonav_timer > 0 then
      resetSpeed()
   end
end

function autonav_end ()
   resetSpeed()
   player.autonavEnd()
end

function autonav_system( sys )
   autonav_setup()
   local dest
   dest, map_npath = player.autonavDest()
   local sysstr
   if sys:known() then
      sysstr = dest:name()
   else
      sysstr = _("Unknown")
   end
   player.msg("#o"..fmt.f(_("Autonav: travelling to {sys}."),{sys=sysstr}).."#0")
   if pilot.canHyperspace(player.pilot()) then
      autonav = autonav_jump_brake
   else
      autonav = autonav_jump_approach
   end
end

function autonav_spob( spb, tryland )
   autonav_setup()
   target_spb = spb
   autonav_tryland = true
   local pos = spb:pos()
   target_pos = pos + vec2.newP( 0.6*spb:radius(), (player.pos()-pos):angle() )

   local spobstr = "#"..spb:colourChar()..spb:name().."#o"
   if tryland then
      player.msg("#o"..fmt.f(_("Autonav: landing on {spob}."),{spob=spobstr}).."#0")
      autonav = autonav_spob_land_approach
   else
      player.msg("#o"..fmt.f(_("Autonav: approaching {spob}."),{spob=spobstr}).."#0")
      autonav = autonav_spob_approach
   end

end

function autonav_pilot( plt )
   autonav_setup()
   target_plt = plt
   local pltstr
   local _inrng, fuzzy = plt:inrange()
   if fuzzy then
      pltstr = _("Unknown")
   else
      pltstr = "#"..plt:colourChar()..plt:name().."#o"
   end

   player.msg("#o"..fmt.f(_("Autonav: following {plt}."),{plt=pltstr}).."#0")
   autonav = autonav_plt_follow
end

function autonav_board( plt )
   autonav_setup()
   local pltstr = "#"..plt:colourChar()..plt:name().."#o"
   player.msg("#o"..fmt.f(_("Autonav: boarding{plt}."),{plt=pltstr}).."#0")
   autonav = autonav_plt_board_approach
end

function autonav_pos( pos )
   autonav_setup()
   player.msg("#o".._("Autonav: heading to target position.").."#0")
   autonav = autonav_pos_approach
   player.autonavSetPos( pos )
   target_pos = pos
end

function autonav_abort( reason )
   if reason then
      player.msg("#r"..fmt.f(_("Autonav: aborted due to '{reason}'!"),{reason=reason}).."#0")
   else
      player.msg("#r".._("Autonav: aborted!").."#0")
   end
   resetSpeed()
end

local function autonav_rampdown( d )
   local pp = player.pilot()
   local speed = pp:stats().speed

   local vel = math.min( 1.5*speed, pp:vel():mod() )
   local t   = d / vel * (1 - 0.075 * tc_base)
   local tint= 3 + 0.5*3*(tc_mod-tc_base)
   if t < tint then
      tc_rampdown = true
      tc_down     = (tc_mod-tc_base) / 3
   end
end

local function autonav_approach( pos, count_target )
   local pp = player.pilot()
   local stats = pp:stats()
   local off = ai.face( pos )
   if off < math.rad(10) then
      ai.accel(1)
   else
      ai.accel(0)
   end

   local speed = stats.speed
   local vmod = pp:vel():mod()
   local t = math.min( 1.5*speed, vmod / stats.thrust )
   local vel = math.min( speed, vmod )
   stats.turn = math.rad(stats.turn) -- TODO probably change the code

   local dist = vel*(t+1.1*math.pi/stats.turn) - 0.5*stats.thrust*t*t

   local d = pos:dist( pp:pos() )

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

local function autonav_follow( pos, vel, follow )
   local pp = player.pilot()
   local stats = pp:stats()

   local timeFactor = math.pi/stats.turn + stats.speed/stats.thrust

   local Kp = 10
   local Kd = math.max( 5, 10.84*timeFactor-10.82 )
   local radius = 100

   if not follow or vel:mod() <= 0 then
      radius = 0
   end

   local angle = math.pi + vel:angle()

   local point = pos + vec2.newP( radius, angle )
   local dir = (point-pp:pos())*Kp + (vel-pp:vel())*Kd

   local off = ai.face( dir )
   if math.abs(off) < math.rad(10) and dir:mod() > 300 then
      ai.accel(1)
   else
      ai.accel(0)
   end

   if not follow then
      return pos:dist( pp:pos() )
   end
end

local function autonav_jump_check ()
   local pp = player.pilot()
   if not pp:navJump() then
      autonav_abort(_("Target changed to current system"))
      return false
   end
   local fuel, consumption = pp:fuel()
   if fuel < consumption then
      autonav_abort(_("Not enough fuel for autonav to continue"))
      return false
   end

   return true
end

function autonav_jump_approach ()
   if not autonav_jump_check() then
      return
   end

   local pp = player.pilot()
   local jmp = pp:navJump()
   if not jmp then
      return autonav_abort()
   end
   local pos = jmp:pos()
   local t = (pp:pos()-jmp:pos()):angle()
   local d = jump:jumpDist()
   pos = pos + vec2.newP( math.max(0.8*d, d-30), t )
   local ret
   ret, d = autonav_approach( pos, false )
   if ret then
      autonav = autonav_jump_brake
   elseif not tc_rampdown and map_npath<=1 then
      autonav_rampdown( d )
   end
end

function autonav_jump_brake ()
   if not autonav_jump_check() then
      return
   end

   local pp = player.pilot()
   local jmp = pp:navJump()
   local ret
   if instant_jump then
      ret = ai.interceptPos( jmp:pos() )
      if not ret and ai.canHyperspace() then
         ret = true
      end
      ai.accel(1)
   else
      local pos = ai.brakeDist()
      if pos:dist( pp:pos() ) > jmp:jumpDist(pp) then
         ret = ai.interceptPos( jmp:pos() )
      else
         ret = ai.brake()
      end
   end

   if ret then
      if ai.canHyperspace() then
         ai.hyperspace()
      end
      autonav = autonav_jump_approach
   end

   if not tc_rampdown and map_npath<=1 then
      tc_rampdown = true
      tc_down     = (tc_mod-tc_base) / 3
   end
end

function autonav_pos_approach ()
   local ret, d = autonav_approach( target_pos, true )
   if ret then
      player.msg("#o".._("Autonav: arrived at position.").."#0")
      return autonav_end()
   elseif not tc_rampdown then
      autonav_rampdown( d )
   end
end

function autonav_spob_approach ()
   local ret, d = autonav_approach( target_spb:pos(), true )
   if ret then
      local spobstr = "#"..target_spb:colourChar()..target_spb:name().."#o"
      if autonav_tryland then
         player.msg("#o"..fmt.f(_("Autonav: landing on {spob}."),{spob=spobstr}).."#0")
      else
         player.msg("#o"..fmt.f(_("Autonav: approaching {spob}."),{spob=spobstr}).."#0")
      end
      return autonav_end()
   elseif not tc_rampdown then
      autonav_rampdown( d )
   end
end

function autonav_spob_land_approach ()
   local ret, d = autonav_approach( target_spb:pos(), true )
   if ret then
      autonav = autonav_spob_land_brake
   elseif not tc_rampdown then
      autonav_rampdown( d )
   end
end

function autonav_spob_land_brake ()
   local ret = ai.brake()
   if ret then
      if player.tryLand(false)=="impossible" then
         autonav_abort()
      else
         autonav = autonav_spob_land_approach
      end
   end

   if not tc_rampdown then
      tc_rampdown = true
      tc_down     = (tc_mod-tc_base) / 3
   end
end

function autonav_plt_follow ()
   local plt = target_plt
   local target_known = false
   local inrng = false
   if plt:exists() then
      local fuzzy
      inrng, fuzzy = plt:inrange()
      target_known = not fuzzy
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

   local canboard = plt:flags("disabled") or plt:flags("boardable")
   local d = autonav_follow( plt:pos(), plt:vel(), not canboard )
   if canboard and not tc_rampdown then
      autonav_rampdown( d )
   end
end

function autonav_plt_board_approach ()
   local plt = target_plt
   local target_known = false
   local inrng = false
   if plt:exists() then
      local fuzzy
      inrng, fuzzy = plt:inrange()
      target_known = not fuzzy
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

   -- TODO
   local ret, d = false, 0
   --local ret, d = autonav_approach_board( plt:pos(), plt:vel(), 50 )
   if not tc_rampdown then
      autonav_rampdown( d )
   end

   -- Finally try to board
   if ret then
      local brd = player.tryBoard(false)
      if brd=="ok" then
         autonav_end()
      elseif brd~="retry" then
         autonav_abort()
      end
   end
end

function autonav_think( dt )
   local _dest
   _dest, map_npath = player.autonavDest()

   if autonav_timer > 0 then
      autonav_timer = autonav_timer - dt
   end

   shouldResetSpeed()

   if autonav then
      autonav()
   end
end

function autonav_update( realdt )
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
