local careful = require "ai.core.misc.careful"
require 'ai.core.idle.generic'
local atk = require "ai.core.attack.util"

-- Keep generic as backup
local idle_generic = idle

-- Get a nearby enemy using pirate heuristics
local function __getenemy ()
   local p, d = atk.preferred_enemy()
   if p and d.v then -- Must be vulnerable
      return p, d.F, d.H
   end
end

-- Sees if there is an enemy nearby to engage
local function __tryengage( p )
   local enemy, F, H = __getenemy()
   if not enemy then
      return false
   end
   local stealth = p:flags("stealth")
   -- Some criterion to determine whether to rambo or try to attack from
   -- stealth
   if not stealth or F*mem.vulnrambo > H then
      ai.stealth(false)
      ai.pushtask( "attack", enemy )
   else
      ai.pushtask( "ambush_stalk", enemy )
   end
   return true
end

-- Tries to loiter in roughly a straight line
local function __loiter( p, taskname )
   local targetdir = mem.lastdirection
   if targetdir then
      -- Low possibility of going in a new random direction
      if rnd.rnd() < 0.1 then
         targetdir = nil
      else
         targetdir = targetdir + rnd.sigma() * math.rad(15)
      end
   end
   local target = careful.getSafePoint( p, nil, nil, nil, targetdir )
   if target then
      local _m, a = (target - p:pos()):polar()
      mem.lastdirection = a -- bias towards moving in a straight line
      ai.pushtask( taskname, target )
      return true
   end
   return false
end

local function idle_leave ()
   -- Get a goal
   if not mem.goal then
      if mem.land_planet and not mem.tookoff then
         local planet = ai.landspob( mem.land_friendly )
         if planet ~= nil then
            mem.goal = "planet"
            mem.goal_planet = planet
            mem.goal_pos = planet:pos()
         end
      end
      if not mem.goal then
         local hyperspace = ai.nearhyptarget()
         if hyperspace then
            mem.goal = "hyperspace"
            mem.goal_hyperspace = hyperspace
            mem.goal_pos = hyperspace:pos()
         end
      end
   end
   if mem.goal then
      if mem.goal == "planet" then
         ai.pushtask("land", mem.goal_planet)
         return true
      elseif mem.goal == "hyperspace" then
         ai.pushtask("hyperspace", mem.goal_hyperspace)
         return true
      end
      mem.goal = nil
   end
   -- Wasn't able to find a goal, just do whatever they were doing
   return false
end

local function idle_nostealth ()
   local p = ai.pilot()

   if mem.force_leave then
      if idle_leave() then return end
   end

   if mem.aggressive then
      local enemy = __getenemy()
      if enemy ~= nil then
         ai.pushtask( "attack", enemy )
         return
      end
   end

   -- Time to leave
   if mem.loiter == 0 then
      if idle_leave() then return end
   end

   -- Get a new point and loiter
   if __loiter( p, "loiter" ) then return end

   -- Fallback to generic
   return idle_generic ()
end

-- Default task to run when idle
function idle ()
   -- Not doing stealth stuff
   if not mem.stealth then
      return idle_nostealth ()
   end

   -- Check stealth and try to stealth
   local p = ai.pilot()
   local stealth = p:flags("stealth")
   if not stealth then
      stealth = ai.stealth()
   end

   -- Check if we want to leave
   if mem.boarded and mem.boarded > 0 then
      if idle_leave() then return end
   end

   -- Just be an asshole if not stealthed and aggressive
   if not stealth and mem.aggressive then
      local enemy = __getenemy()
      if enemy ~= nil then
         ai.pushtask( "attack", enemy )
         return
      end
   end

   -- If not stealth, just do normal pirate stuff
   if not stealth then
      return idle_generic() -- TODO something custom
   end

   -- TODO non-aggressive behaviours
   --if not mem.aggressive then
   --end

   -- See if there is a nearby target to kill
   if __tryengage(p) then return end

   -- Just move around waiting for ambush
   if __loiter( p, "ambush_moveto" ) then return end

   -- Wasn't able to find out what to do, so just fallback to no stealth...
   return idle_nostealth()
end

-- Try to back off from the target
function backoff( target )
   if not target or not target:exists() then
      ai.poptask()
      return
   end

   -- Target distance to get to
   local tdist
   if mem.stealth then
      tdist = mem.ambushclose
   else
      tdist = mem.enemyclose
   end
   tdist = tdist or 3000
   tdist = tdist * 1.5

   local p = ai.pilot()

   -- Get away
   local dir = ai.face( target, true )
   ai.accel()

   -- Afterburner handling.
   if ai.hasafterburner() and p:energy() > 30 then
      ai.weapset( 8, true )
   end
   if mem._o and dir < math.rad(25) then
      if mem._o.blink_drive then
         p:toggleOutfit( mem._o.blink_drive, true )
      elseif mem._o.blink_engine then
         p:toggleOutfit( mem._o.blink_engine, true )
      end
   end

   -- When out of range pop task
   if ai.dist2( target ) > math.pow(tdist,2) then
      ai.poptask()
      return
   end
end

control_funcs.ambush_moveto = function ()
   -- Try to engage hostiles
   __tryengage( ai.pilot() )
   return true
end
control_funcs.ambush_stalk = function ()
   local p = ai.pilot()
   local target = ai.taskdata()
   if not target or not target:exists() then
      ai.poptask()
      return
   end

   -- Make sure target is not too far away
   if mem.ambushclose and ai.dist2(target) > math.pow(2*mem.ambushclose,2) then
      ai.poptask()
      return
   end
   -- Ignore enemies that are in safe zone again
   if not careful.posIsGood( p, target:pos() ) then
      ai.poptask()
      return
   end
end
control_funcs.attack = function ()
   -- Ignore non-vulnerable targets
   local target = ai.taskdata()
   if not target or not target:exists() then
      ai.poptask()
      return false
   end

   local p = ai.pilot()
   if not careful.checkVulnerable( p, target, mem.vulnabort ) then
      ai.poptask()

      -- Try to get a new enemy
      local enemy = __getenemy()
      if enemy ~= nil then
         ai.pushtask( "attack", enemy )
      else
         ai.pushtask( "backoff", target )
      end
      return true
   end

   return control_funcs.generic_attack()
end
control_funcs.inspect_moveto = function ()
   local p = ai.pilot()
   local target = ai.taskdata()
   if mem.natural and target and not careful.posIsGood( p, target ) then
      ai.poptask()
      return false
   end
   return true
end

-- Custom should attack
local should_attack_generic = should_attack
function should_attack( enemy, si )
   if not enemy then return false end

   local p = ai.pilot()
   -- If stealthed we don't want to attack normally
   local stealth = p:flags("stealth")
   if stealth then return false end

   -- Do normal check
   local res = should_attack_generic( enemy, si )
   if not res then return false end

   -- Make sure vulnerable
   if not careful.checkVulnerable( p, enemy, mem.vulnattack ) then
      return false
   end
   return true
end

-- Try to investigate
function should_investigate( pos, _si )
   if careful.posIsGood( ai.pilot(), pos ) then
      return false
   elseif ai.dist2(pos) < (mem.enemyclose or math.huge) then
      return true
   end
   return false
end

-- Settings
mem.doscans       = false
mem.loiter        = math.huge -- They loiter until they can steal!
mem.stealth       = true
mem.aggressive    = true -- Pirates are aggressive
mem.lanedistance  = 2000
mem.enemyclose    = nil
mem.ambushclose   = nil
mem.vulnrange     = 3000
mem.vulnrambo     = 1.0
mem.vulnattack    = 1.5 -- Vulnerability threshold to attack (higher is less vulnerable)
mem.vulnabort     = 2.0 -- Vulnerability threshold to break off attack (lower is more vulnerable)
mem.vulnignore    = false
mem.atk_pref_func = atk.prefer_weaker -- Pirates prefer weaker targets
