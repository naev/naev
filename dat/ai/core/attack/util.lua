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
--Attempts to maintain a constant distance from nearby things
--This modulates the distance between the current pilot and its nearest neighbor
--]]
function atk.keep_distance()
   --anticipate this will be added to eliminate potentially silly behavior if it becomes a problem
   --local flight_offset = ai.drift_facing()

   --find nearest thing
   local neighbor = ai.nearestpilot()
   if not neighbor or not neighbor:exists() then
      return
   end

   --find the distance based on the direction I'm travelling
   local perp_distance = ai.flyby_dist(neighbor)
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
   -- Some AI will not do fancy maneuvers
   if mem.simplecombat then return false end
   if target:flags("disabled") then return false end -- Don't be fance with disabled ships
   -- The situation is the following: we're out of range, facing the target,
   -- going towards the target, and someone is shooting on us.
   local pilot  = ai.pilot()
   local range  = ai.getweaprange(3)
   local dir    = ai.idir(target)

   local _m1, d1 = vec2.polar( pilot:vel() )
   local _m2, d2 = vec2.polar( target:pos() - pilot:pos() )
   local d = d1-d2

   return ( (dist > range) and (ai.hasprojectile())
           and (dir < math.rad(10)) and (dir > -math.rad(10)) and (d < math.rad(10)) and (d > -math.rad(10)) )
end

--[[
-- Tries to shoot seekers at close range
--]]
function atk.dogfight_seekers( dist, dir )
   if dist > 100 then
      if dist < ai.getweaprange( 4 ) and dir < math.rad(20)  then
         ai.weapset( 4 )
      elseif dist < ai.getweaprange( 9 ) then
         ai.weapset( 9 )
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
   if not mem.atk_kill and target:flags("disabled") then
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

   local enemy = atk.preferred_enemy( atk.prefer_capship )

   local dist = ai.dist(target)
   local range = ai.getweaprange(3, 0)
   -- Get new target if it's closer
   -- prioritize targets within the size limit
   if enemy ~= target and enemy ~= nil then
      -- Shouldn't switch targets if close
      if dist > range * mem.atk_changetarget then
         ai.pushtask("attack", enemy )
      end
      return
   end

   local nearest_enemy = ai.getenemy()
   if nearest_enemy ~= target and nearest_enemy ~= nil then
      -- Shouldn't switch targets if close
      if dist > range * mem.atk_changetarget then
         ai.pushtask("attack", nearest_enemy )
      end
   end
end

--[[
-- Execute a sequence of close-in flyby attacks
-- Uses a combination of facing and distance to determine what action to take
-- This version is slightly less aggressive and cruises by the target
--]]
function atk.flyby( target, dist )
   local range = ai.getweaprange(3)
   local dir
   ai.weapset( 3 ) -- Forward/turrets

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
      if dir < math.rad(10) then
         ai.shoot()
      end
      ai.shoot(true)

      -- Also try to shoot missiles
      atk.dogfight_seekers( dist, dir )
   end
end

--[[
-- Attack Profile for a maneuverable ship engaging a maneuverable target
--
--This is designed for fighters engaging other fighters
--]]
function atk.space_sup( target, dist )
   local range = ai.getweaprange(3)
   local dir
   ai.weapset( 3 ) -- Forward/turrets

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
      if dir < math.rad(10) then
         ai.shoot()
      end
      ai.shoot(true)

      -- Also try to shoot missiles
      atk.dogfight_seekers( dist, dir )

   --within close range; aim and blast away with everything
   else
      dir = ai.aim(target)
      -- Shoot if should be shooting.
      if dir < math.rad(10) then
         ai.shoot()
      end
      ai.shoot(true)
   end
end

local function ___atk_g_ranged_dogfight( target, dist )
   local dir
   if not mem.careful or dist < 3 * ai.getweaprange(3, 0) * mem.atk_approach then
      dir = ai.face(target) -- Normal face the target
   else
      dir = ai.careful_face(target) -- Careful method
   end

   -- Check if in range to shoot missiles
   if dist < ai.getweaprange( 4 ) then
      if dir < math.rad(30) then
         ai.weapset( 4 ) -- Weaponset 4 contains weaponset 9
      else
         ai.weapset( 9 )
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
      ai.weapset( 3 ) -- Set turret/forward weaponset.
      ai.shoot()
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
   local range = ai.getweaprange( 4 )
   range = math.min ( range - dist * radial_vel / ( ai.getweapspeed( 4 ) - radial_vel ), range )

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
         ai.weapset( 4 )
         -- If he managed to shoot, reinitialize the timer
         if ai.shoot_indicator() and not ai.timeup(1) then
            ai.settimer(1, 13.0)
         end
      end
   end

   -- We didn't shoot with all seekers: see if it's appropriate to use turreted ones
   if (not shoot4) then
      range = ai.getweaprange( 9 )
      range = math.min ( range - dist * radial_vel / ( ai.getweapspeed( 9 ) - radial_vel ), range )
      if dist < range*0.95 then
         ai.weapset( 9 )
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
   local range = ai.getweaprange( 4 )

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
         ai.weapset( 4 )
      end

      if dir < math.rad(10) then
         ai.weapset( 3 ) -- Set turret/forward weaponset.
         ai.shoot()
      end
      ai.shoot(true)
   end
end
--[[
-- Enters ranged combat with the target
--]]
function atk.ranged( target, dist )
   local range = ai.getweaprange( 4 )
   local wrange = math.min( ai.getweaprange(3,0), ai.getweaprange(3,1) )

   -- Pilot thinks dogfight is the best
   if ai.relhp(target)*ai.reldps(target) >= 0.25
         or ai.getweapspeed(4) < target:speedMax()*1.2
         or range < ai.getweaprange(1)*1.5 then
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

   -- Always launch fighters for now
   ai.weapset( 5 )
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
function atk.preferred_enemy( pref_func )
   pref_func = mem.atk_pref_func or pref_func or atk.prefer_similar
   local p = ai.pilot()
   local r = math.pow( mem.lanedistance, 2 )
   local targets = {}
   for k,h in ipairs( p:getEnemies( mem.enemyclose, nil, false, false, true ) ) do
      local w = h:memory().vulnerability or 0
      if w < math.huge then -- math.huge can be used to make the AI try not to target
         local v, F, H = careful.checkVulnerable( p, h, mem.vulnattack, r )
         if not v then
            F = 1
            H = 1
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
