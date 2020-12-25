-- Default task to run when idle
function idle ()
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
      sysrad = rnd.rnd() * system.cur():radius()
      angle = rnd.rnd() * 2 * math.pi
      ai.pushtask("__moveto_nobrake", vec2.new(math.cos(angle) * sysrad, math.sin(angle) * sysrad))
   end
   mem.loiter = mem.loiter - 1
end

-- Settings
mem.land_friendly = true -- Land on only friendly by default
