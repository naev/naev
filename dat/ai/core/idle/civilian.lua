local lanes = require 'ai.core.misc.lanes'

-- Default task to run when idle
function idle ()
   if mem.loiter == nil then mem.loiter = 3 end
   if mem.loiter == 0 then -- Try to leave. Civilians will always try to land on a planet if there is one.
       local planet = ai.landplanet( mem.land_friendly )
       -- planet must exist
       if planet == nil then
          ai.settimer(0, rnd.int(1000, 3000))
          ai.pushtask("enterdelay")
       else
          mem.land = planet:pos()
          ai.pushtask("hyperspace")
          ai.pushtask("land")
       end
   else -- Stay. Have a beer.
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
