local lanes = require 'ai.core.misc.lanes'
require 'ai.core.idle.generic'

-- Keep generic as backup
idle_generic = idle

function __getenemy( p )
   local pv = {}
   local r = math.pow( mem.lanedistance, 2 )
   for k,v in ipairs(p:getHostiles( mem.ambushclose )) do
      -- Make sure not in safe lanes
      if lanes.getDistance2( v:pos() ) > r then
         local d  = ai.dist2( v )
         table.insert( pv, {p=v, d=d} )
      end
   end
   table.sort( pv, function(a,b)
      return a.d < b.d
   end )

   if #pv==0 then
      return nil
   end
   return pv[1].p
end

function idle_leave ()
   -- Get a goal
   if not mem.goal then
      if mem.land_planet and not mem.tookoff then
         local planet = ai.landplanet( mem.land_friendly )
         if planet ~= nil then
            mem.goal = "planet"
            mem.goal_planet = planet
            mem.goal_pos = planet:pos()
            mem.land = mem.goal_pos
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
         ai.push("land")
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

function idle_nostealth ()
   local p = ai.pilot()

   if mem.aggressive then
      local enemy = __getenemy(p)
      if enemy ~= nil and (not mem.ambushclose or ai.dist(enemy) < mem.ambushclose) then
         ai.pushtask( "attack", enemy )
         return
      end
   end

   -- Time to leave
   if mem.loiter == 0 then
      if idle_leave() then return end
   end

   -- Get a new point and loiter
   local target = lanes.getNonPoint()
   if target then
      ai.pushtask( "loiter", target )
      mem.loiter = mem.loiter - 1
      return
   end

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
      local enemy  = __getenemy(p)
      if enemy ~= nil and (not mem.ambushclose or ai.dist(enemy) < mem.ambushclose) then
         ai.pushtask( "attack", enemy )
         return
      end
   end

   -- If not stealth, just do normal pirate stuff
   if not stealth then
      return idle_generic() -- TODO something custom
   end

   if not mem.aggressive then
      -- TODO non-aggressive behaviours
   end

   -- See if there is a nearby target to kill
   local enemy = __getenemy(p)
   if enemy ~= nil and (not mem.ambushclose or ai.dist(enemy) < mem.ambushclose) then
      ai.pushtask( "ambush_stalk", enemy )
      return
   end

   -- Just move around waiting for ambush
   local target = lanes.getNonPoint()
   if target then
      ai.pushtask( "ambush_moveto", target )
      return
   end

   -- Wasn't able to find out what to do, so just fallback to no stealth...
   return idle_nostealth()
end

-- Settings
mem.doscans       = false
mem.loiter        = math.huge -- They loiter until they can steal!
mem.stealth       = true
mem.aggressive    = true -- Pirates are aggressive
mem.lanedistance  = 2000
mem.enemyclose    = 1000 -- Don't aggro normally unless very close
mem.ambushclose   = nil
