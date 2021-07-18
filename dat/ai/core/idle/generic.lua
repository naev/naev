local lanes = require 'ai.core.misc.lanes'

-- Default task to run when idle
function idle ()
   -- Aggressives will try to find enemies first, before falling back on
   -- loitering, to avoid weird stuff starting to scan before attacking
   if mem.aggressive then
      local enemy  = ai.getenemy()
      if enemy ~= nil then
         ai.pushtask( "attack", enemy )
         return
      end
   end

   if mem.loiter == nil then mem.loiter = 3 end
   if mem.loiter == 0 then -- Try to leave.
      local planet = ai.landplanet( mem.land_friendly )
      -- planet must exist
      if planet == nil or mem.land_planet == false then
         ai.settimer(0, rnd.int(1000, 3000))
         ai.pushtask("enterdelay")
      else
         mem.land = planet:pos()
         ai.pushtask("hyperspace")
         if not mem.tookoff then
            ai.pushtask("land")
         end
      end
   else -- Stay. Have a beer.
      if mem.doscans then
         local target = __getscantarget()
         if target then
            __push_scan( target )
            return
         end
      end

      -- Check to see if we want to patrol waypoints
      -- TODO should waypoints use the mem.loiter also?
      if mem.waypoints then
         -- If haven't started patroling, find the closest waypoint
         if not mem._waypoint_cur then
            local dist = math.huge
            local closest = nil
            for k,v in pairs(mem.waypoints) do
               local vd = ai.dist( v )
               if vd < dist then
                  dist = vd
                  closest = k
               end
            end
            mem._waypoint_cur = closest
         else
            mem._waypoint_cur = math.mod( mem._waypoint_cur, #mem.waypoints )+1
         end
         -- Go to the next position
         ai.pushtask( "loiter", mem.waypoints[ mem._waypoint_cur ] )
      else
         -- Go to an interesting
         if not mem.route then
            local target = lanes.getPointInterest()
            mem.route = lanes.getRoute( target )
         end
         local pos = mem.route[1]
         table.remove( mem.route, 1 )
         if #mem.route == 0 then
            mem.loiter = mem.loiter - 1
            mem.route = nil
         end
         ai.pushtask("loiter", pos + vec2.newP(200*rnd.rnd(),360*rnd.rnd()) )
      end
   end
end


-- Settings
mem.waypoints     = nil
mem.land_friendly = false -- Land on only friendly by default
mem.doscans       = false
