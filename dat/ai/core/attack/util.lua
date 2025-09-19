--[[
--    Attack utilitiesGeneric attack functions
--]]
local scans = require "ai.core.misc.scans"
local careful = require "ai.core.misc.careful"

local atk = {}

mem.lanedistance = mem.lanedistance or mem.enemyclose or 3e3
--mem.atk_pref_func = nil
mem.atk_pref_range = 5e3 -- Range to prefer to attack from

--[[
0 -> all weaps (switch, primary=primary, secondary=secondary)
1 -> forward (switch)
2 -> turret (switch)
3 -> all_weaps (switch, primary=forward, secondary=turret)
4 -> seekers (contains 9)
5 -> fighter bays
6 -> ()
7 -> afterburner
8 -> ()
9 -> turret seekers
--]]

atk.PRIMARY = 1
atk.SECONDARY = 2
atk.POINTDEFENSE = 3
atk.FIGHTERBAYS = 4
atk.TURRETS = 5
atk.SEEKERS = 6
atk.SEEKER_TURRETS = 7

function atk.shootall()
   atk.primary()
   atk.secondary()
end
function atk.primary()
   ai.weapset( 1, true ) -- Usually beam / bolt are here
end
function atk.secondary()
   ai.weapset( 2, true ) -- Usually seekers / ammo outfits are here
end
function atk.pointdefence()
   ai.weapset( 3, true ) -- Probably want to keep on most of the time
end
function atk.fighterbays()
   ai.weapset( 4, true ) -- Toggle to launch
end
function atk.fb_and_pd()
   atk.pointdefence()
   atk.fighterbays()
end
function atk.turrets()
   ai.weapset( 5, true )
end
function atk.seekers()
   ai.weapset( 6, true )
end
function atk.seeker_turrets()
   ai.weapset( 7, true )
end

function atk.primary_range()
   return ai.getweaprange( 1 )
end
function atk.secondary_range()
   return ai.getweaprange( 2 )
end
function atk.turrets_range()
   return ai.getweaprange( 5 )
end
function atk.seekers_range()
   return ai.getweaprange( 6 )
end
function atk.seeker_turrets_range()
   return ai.getweaprange( 7 )
end

function atk.seekers_speed()
   return ai.getweapspeed( 6 )
end
function atk.seeker_turrets_speed()
   return ai.getweapspeed( 7 )
end

function atk.seekers_ammo()
   return ai.getweapammo( 6 )
end

--[[
--Attempts to maintain a constant distance from nearby things
--This modulates the distance between the current pilot and its nearest neighbour
--]]
function atk.keep_distance()
   --anticipate this will be added to eliminate potentially silly behaviour if it becomes a problem
   --local flight_offset = ai.drift_facing()

   --find nearest thing
   local neighbour = ai.nearestpilot()
   if not neighbour or not neighbour:exists() then
      return
   end

   --find the distance based on the direction I'm travelling
   local perp_distance = ai.flyby_dist(neighbour)
   -- adjust my direction of flight to account for this
   -- if pilot is too close, turn away
   if perp_distance < 0 and perp_distance > -50 then
      ai.turn(1)
   elseif perp_distance > 0 and perp_distance < 50 then
      ai.turn(-1)
   end
end

--[[
-- Tests if the target is seeable (even fuzzy). If no, try to go where it was last seen
-- This is not supra-clean as we are not supposed to know [target:pos()]
-- But the clean way would require to have stored the target position into memory
-- This test should be put in any subtask of the attack task.
--]]
function atk.check_seeable( target )
   if scans.check_visible( target ) then
      return true
   end
   scans.investigate( target )
   return false
end

--[[
-- Decides if zigzag is a good option
--]]
function atk.decide_zz( target, dist )
   -- Some AI will not do fancy manoeuvres
   if mem.atk_skill <= 0.45+0.55*mem.rand then return false end
   if target:disabled() then return false end -- Don't be fance with disabled ships
   -- The situation is the following: we're out of range, facing the target,
   -- going towards the target, and someone is shooting on us.
   local pilot  = ai.pilot()
   local range  = atk.primary_range()
   local dir    = ai.idir(target)

   local _m1, d1 = vec2.polar( pilot:vel() )
   local _m2, d2 = vec2.polar( target:pos() - pilot:pos() )
   local d = d1-d2

   return ( (dist > range) and (ai.hasprojectile())
      and (dir < math.rad(10)) and (dir > -math.rad(10))
      and (d < math.rad(10)) and (d > -math.rad(10)) )
end

--[[
-- Tries to shoot seekers at close range
--]]
function atk.dogfight_seekers( dist, dir )
   if dist > 100 then
      if dist < atk.seekers_range() and dir < math.rad(20)  then
         atk.seekers()
      elseif dist < atk.seeker_turrets_range() then
         atk.seeker_turrets()
      end
   end
end

--[[
-- Common control stuff
--]]
function atk.com_think( target, dokill )
   -- make sure pilot exists
   if not target or not target:exists() then
      ai.poptask()
      return
   end

   -- Check if is bribed by target
   if ai.isbribed(target) then
      ai.poptask()
      return
   end

   -- Kill the target
   if dokill then
      return target
   end

   -- Check if we want to board
   if mem.atk_board and ai.canboard(target) then
      ai.pushtask("board", target )
      return
   end

   -- Check to see if target is disabled
   if not mem.atk_kill and target:disabled() then
      ai.poptask()
      return
   end

   return target
end

--[[
-- big game hunter attack pattern using heuristic target identification.
--]]
function atk.heuristic_big_game_think( target, _si )
   -- Chance to just focus on the current enemy
   if rnd.rnd() < 0.7 then
      return
   end

   -- Don't switch targets if close to current one
   local dist = ai.dist(target)
   local range = atk.primary_range()
   if dist < range * mem.atk_changetarget then
      return
   end

   -- Prioritize preferred target
   local enemy = atk.preferred_enemy( atk.prefer_capship )
   if enemy ~= target and enemy ~= nil then
      ai.pushtask("attack", enemy )
      return
   end
end

--[[
-- Execute a sequence of close-in flyby attacks
-- Uses a combination of facing and distance to determine what action to take
-- This version is slightly less aggressive and cruises by the target
--]]
function atk.flyby( target, dist )
   local range = atk.primary_range()
   local dir

   -- First test if we should zz
   if atk.decide_zz( target, dist ) then
      ai.pushsubtask("_attack_zigzag", target)
   end

   -- Far away, must approach
   if dist > (3 * range) then
      dir = ai.idir(target)
      if dir < math.rad(10) and dir > -math.rad(10) then
         atk.keep_distance()
         ai.accel()
      else
         ai.iface(target)
      end

   -- Midrange
   elseif dist > (0.75 * range) then
      dir = ai.idir(target)
      --test if we're facing the target. If we are, keep approaching
      if dir <= math.rad(30) and dir > -math.rad(30) then
         ai.iface(target)
         if dir < math.rad(10) and dir > -math.rad(10) then
            ai.accel()
         end
      elseif dir > math.rad(30) and dir < math.pi then
         ai.turn(1)
         ai.accel()
      else
         ai.turn(-1)
         ai.accel()
      end

   --otherwise we're close to the target and should attack until we start to zip away
   else
      dir = ai.aim(target)
      --not accelerating here is the only difference between the aggression levels. This can probably be an aggression AI parameter
      if mem.aggressive == true then
         ai.accel()
      end

      -- Shoot if should be shooting.
      atk.primary()
      atk.secondary()

      -- Also try to shoot missiles
      atk.dogfight_seekers( dist, dir )
   end

   -- Always launch fighters for now
   atk.fb_and_pd()
end

--[[
-- Attack Profile for a manoeuvrable ship engaging a manoeuvrable target
--
--This is designed for fighters engaging other fighters
--]]
function atk.space_sup( target, dist )
   local range = atk.primary_range()
   local dir

   -- First test if we should zz
   if atk.decide_zz( target, dist ) then
      ai.pushsubtask("_attack_zigzag", target)
   end

   --if we're far away from the target, then turn and approach
   if dist > (range) then
      dir = ai.idir(target)
      if dir < math.rad(10) and dir > -math.rad(10) then
         atk.keep_distance()
         ai.accel()
      else
         ai.iface(target)
      end

   elseif dist > 0.8* range then
      --drifting away from target, so emphasize intercept
      --course facing and accelerate to close
      dir = ai.iface(target)
      if dir < math.rad(10) and dir > -math.rad(10) then
         ai.accel()
      end

   --within close range; aim and blast away with everything
   elseif dist > 0.4*range then
      dir = ai.aim(target)
      local dir2 = ai.idir(target)

      --accelerate and try to close
      --but only accel if it will be productive
      if dir2 < math.rad(15) and dir2 > -math.rad(15) and ai.relvel(target) > -math.rad(10) then
         ai.accel()
      end

      -- Shoot if should be shooting.
      atk.shootall()

      -- Also try to shoot missiles
      atk.dogfight_seekers( dist, dir )

   --within close range; aim and blast away with everything
   else
      ai.aim(target)
      atk.shootall()
   end

   -- Always launch fighters and pd for now
   atk.fb_and_pd()
end

local function ___atk_g_ranged_dogfight( target, dist )
   local dir
   if mem.atk_skill <= 0.45+0.55*mem.rand or dist < 3 * atk.primary_range() * mem.atk_approach then
      dir = ai.face(target) -- Normal face the target
   else
      dir = ai.careful_face(target) -- Careful method
   end

   -- Check if in range to shoot missiles
   if dist < atk.seekers_range() then
      if dir < math.rad(30) then
         atk.seekers()
      else
         atk.seeker_turrets()
      end
   else
      -- Test if we should zz
      if ai.pilot():mass() < 400 and atk.decide_zz( target, dist ) then
         ai.pushsubtask("_attack_zigzag", target)
      end
   end

   -- Approach for melee
   if dir < math.rad(10) then
      ai.accel()
      atk.primary()
   end
end
local function ___atk_g_ranged_strafe( target, dist )
--[[ The pilot tries first to place himself at range and at constant velocity.
      When he is stabilized, he starts shooting until he has to correct his trajectory again

      If he doesn't manage to shoot missiles after a few seconds
      (because the target dodges),
      he gives up and just faces the target and shoot (provided he is in range)
]]

   local p = ai.pilot()

   -- Estimate the range
   local radial_vel = ai.relvel(target, true)
   local range = atk.seekers_range()
   range = math.min ( range - dist * radial_vel / ( atk.seekers_speed() - radial_vel ), range )

   local goal = ai.follow_accurate(target, range * 0.8, 0, 10, 20, "keepangle")
   local mod = vec2.mod(goal - p:pos())

   local shoot4 = false -- Flag to see if we shoot with all seekers

   --Must approach or stabilize
   if mod > 3000 then
      -- mustapproach allows a hysteretic behaviour
      mem.mustapproach = true
   end
   if dist > range*0.95 then
      mem.outofrange = true
   end

   if (mem.mustapproach and not ai.timeup(1) ) or mem.outofrange then
      local dir   = ai.face(goal)
      if dir < math.rad(10) and mod > 300 then
         ai.accel()
         --mem.stabilized = false
      -- ship must be stabilized since 2 secs
      elseif ai.relvel(target) < 5 and not ai.timeup(1) then--[[if not mem.stabilized then
         mem.stabilized = true
         ai.settimer(0, 2.0)
      elseif not ai.timeup(1) and ai.timeup(0) then
         -- If the ship manages to catch its mark, reset the timer]]
         --ai.settimer(1, 10.0)
         mem.mustapproach = false
      end
      if dist < range*0.85 then
         mem.outofrange = false
      end

   else -- In range
      local dir  = ai.face(target)
      if dir < math.rad(30) then
         ai.set_shoot_indicator(false)
         atk.seekers()
         -- If he managed to shoot, reinitialize the timer
         if ai.shoot_indicator() and not ai.timeup(1) then
            ai.settimer(1, 13.0)
         end
      end
   end

   -- We didn't shoot with all seekers: see if it's appropriate to use turreted ones
   if not shoot4 then
      range = atk.seeker_turrets_range()
      range = math.min ( range - dist * radial_vel / ( atk.seeker_turrets_speed() - radial_vel ), range )
      if dist < range*0.95 then
         atk.seeker_turrets()
      end
   end

   --The pilot just arrived in the good zone :
   --From now, if ship doesn't manage to stabilize within a few seconds, shoot anyway
   if dist < 1.5*range and not mem.inzone then
      mem.inzone = true
      ai.settimer(1, mod/p:speed()*0.7 )
   end
end
local function ___atk_g_ranged_kite( target, dist )
   local p = ai.pilot()

   -- Estimate the range
   local range = atk.seekers_range()

   -- Try to keep velocity vector away from enemy
   local targetpos = target:pos()
   local selfpos = p:pos()
   local _unused, targetdir = (selfpos-targetpos):polar()
   local velmod, veldir = p:vel():polar()
   if velmod < 0.8*p:speed() or math.abs(targetdir-veldir) > math.rad(30) then
      local dir = ai.face( target, true )
      if math.abs(math.pi-dir) < math.rad(30) then
         ai.accel()
      end
      return
   end

   -- We are flying away, so try to kite
   local dir = ai.aim(target) -- aim instead of facing
   if dir < math.rad(30) then
      if dist < range*0.95 then
         atk.seekers()
      end
      atk.primary()
   end
end
--[[
-- Enters ranged combat with the target
--]]
function atk.ranged( target, dist )
   local range = atk.seekers_range()
   local wrange = math.min( atk.primary_range(), atk.secondary_range() )

   -- Pilot thinks dogfight is the best
   if ai.relhp(target)*ai.reldps(target) >= 0.25
         or atk.seekers_speed() < target:speedMax()*1.2
         or range < atk.primary_range()*1.5 then
      ___atk_g_ranged_dogfight( target, dist )
   elseif target:target()==ai.pilot() and dist < range and ai.hasprojectile() then
      local tvel = target:vel()
      local pvel = ai.pilot():vel()
      local vel = (tvel-pvel):polar()
      -- If will make contact soon, try to engage
      if dist < wrange+8*vel then
         ___atk_g_ranged_dogfight( target, dist )
      else
      -- Getting chased, try to kite
         ___atk_g_ranged_kite( target, dist )
      end
   else
      -- Enemy is distracted, try to strafe and harass without engaging
      ___atk_g_ranged_strafe( target, dist )
   end

   -- Always launch fighters and pd for now
   atk.fb_and_pd()
end

function atk.prefer_similar( p, h, v )
   local w = math.abs( p:points() - h:points() ) -- Similar in points
   w = w + 50 / math.pow( mem.atk_pref_range, 2 ) * p:pos():dist2( h:pos() ) -- Squared distance normalized to 1
   -- Bring down vulnerability a bit
   if not v then
      w = w + 100
   end
   return w
end

function atk.prefer_capship( p, h, v )
   local w = -math.min( 100, h:points() ) -- Random threshold
   -- distance is less important to capships
   w = w + 10 / math.pow( mem.atk_pref_range, 2 ) * p:pos():dist2( h:pos() )
   -- Bring down vulnerability a bit
   if not v then
      w = w + 100
   end
   return w
end

function atk.prefer_weaker( p, h, v )
   local w = math.max( 0, h:points() - p:points() ) -- penalize if h has more points
   w = w + 50 / math.pow( mem.atk_pref_range, 2 ) * p:pos():dist2( h:pos() ) -- Squared distance normalized to 1
   -- Bring down vulnerability a bit
   if not v then
      w = w + 100
   end
   return w
end

--[[
Evaluates a single enemy
--]]
function atk.preferred_enemy_test( target, pref_func )
   pref_func = mem.atk_pref_func or pref_func or atk.prefer_similar
   local p = ai.pilot()
   local w = target:memory().vulnerability or 0
   local r = math.pow( mem.lanedistance, 2 )
   if w < math.huge then -- math.huge can be used to make the AI try not to target
      local v, F, H = careful.checkVulnerable( p, target, mem.vulnattack, r )
      if not v then
         F = 1
         H = 1
      end
      -- Insert some randomness for less consistency
      w = w + (0.9+0.2*rnd.rnd())*pref_func( p, target, v, F, H ) -- Compute pref function
   end
   return w
end

--[[
Tries to find a preferred enemy.
--]]
function atk.preferred_enemy( pref_func, checkvuln )
   pref_func = mem.atk_pref_func or pref_func or atk.prefer_similar
   local p = ai.pilot()
   local r = math.pow( mem.lanedistance, 2 )
   local targets = {}
   for k,h in ipairs( p:getEnemies( mem.enemyclose, nil, false, false, true ) ) do
      local w = h:memory().vulnerability or 0
      if w < math.huge then -- math.huge can be used to make the AI try not to target
         local v, F, H
         if not checkvuln then
            v = true
            F = 1
            H = 1
         else
            v, F, H = careful.checkVulnerable( p, h, mem.vulnattack, r )
            if not v then
               F = 1
               H = 1
            end
         end
         -- Insert some randomness for less consistency
         w = w + (0.9+0.2*rnd.rnd())*pref_func( p, h, v, F, H ) -- Compute pref function
         table.insert( targets, { p=h, priority=w, v=v, F=F, H=H } )
      end
   end
   if #targets <= 0 then return nil end
   table.sort( targets, function(a,b)
      return a.priority < b.priority -- minimizing
   end )
   -- TODO statistical sampling instead of determinism?
   -- Some randomness is handled above so it may not be necessary
   return targets[1].p, targets[1]
end

return atk
