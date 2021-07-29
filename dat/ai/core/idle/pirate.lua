local lanes = require 'ai.core.misc.lanes'
require 'ai.core.idle.generic'

-- Keep generic as backup
idle_generic = idle

-- Default task to run when idle
function idle ()
   -- Not doing ambushes
   if not mem.ambush then
      return idle_generic()
   end

   -- Check stealth and try to stealth
   local p = ai.pilot()
   local stealth = p:flags("stealth")
   if not stealth then
      stealth = ai.stealth()
   end

   -- Check if we want to leave
   if mem.boarded and mem.boarded > 0 then
      -- Get a goal
      if not mem.goal then
         if mem.land_planet and not mem.tookoff then
            local planet = ai.landplanet( mem.land_friendly )
            if planet ~= nil then
               mem.goal = "planet"
               mem.goal_planet = planet
               mem.goal_pos = planet:pos()
               mem.land = mem.goal_pos
               ai.pushtask("land")
               return
            end
         end
         if not mem.goal then
            local hyperspace = ai.nearhyptarget()
            if hyperspace then
               mem.goal = "hyperspace"
               mem.goal_hyperspace = hyperspace
               mem.goal_pos = hyperspace:pos()
               ai.pushtask("hyperspace", mem.goal_hyperspace)
               return
            end
         end
         -- Wasn't able to find a goal, just do whatever they were doing
      end
   end

   -- Just be an asshole if not stealthed and aggressive
   if not stealth and mem.aggressive then
      local enemy  = ai.getenemy()
      if enemy ~= nil and (not mem.enemyclose or ai.dist(enemy) < mem.enemyclose) then
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
   local enemy = ai.getenemy()
   if enemy ~= nil and (not mem.enemyclose or ai.dist(enemy) < mem.enemyclose) then
      ai.pushtask( "ambush_stalk", enemy )
      return
   end

   -- Just move around waiting for ambush
   local target = lanes.getNonPoint()
   if target then
      ai.pushtask( "ambush_moveto", target )
      return
   end

   -- Wasn't able to find out what to do, so just fallback to generic again...
   return idle_generic()
end

-- Settings
mem.loiter        = 3
mem.ambush        = true
mem.aggressive    = true -- Pirates are aggressive
mem.lanedistance  = nil
