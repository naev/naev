-- Default task to run when idle. Similar as civilian, but with a message
function idle ()
   ai.pilot():broadcast(mem.ad)
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
      sysrad = rnd.rnd() * system.cur():radius()
      angle = rnd.rnd() * 2 * math.pi
      ai.pushtask("__goto_nobrake", vec2.new(math.cos(angle) * sysrad, math.sin(angle) * sysrad))
   end
   mem.loiter = mem.loiter - 1
end
