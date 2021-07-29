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
