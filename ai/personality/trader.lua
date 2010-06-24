-- Default task to run when idle
function idle ()
   local planet = ai.landplanet( mem.land_friendly )
   -- planet must exist.
   if planet == nil or mem.land_planet == false then
      ai.settimer(0, rnd.int(1000, 3000))
      ai.pushtask("enterdelay")
   else
      mem.land = planet
      ai.pushtask("hyperspace")
      if not mem.tookoff then
         ai.pushtask("land")
      end
   end
end
