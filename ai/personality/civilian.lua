-- Default task to run when idle
function idle ()
   r = rnd.rnd()
   if r < 0.25 then -- Try to leave. Civilians will always try to land on a planet if there is one.
       local planet = ai.landplanet()
       -- planet must exist
       if planet == nil then
          ai.settimer(0, rnd.int(1000, 3000))
          ai.pushtask("enterdelay")
       else
          mem.land = planet
          ai.pushtask("hyperspace")
          ai.pushtask("land")
       end
   else -- Stay. Have a beer.
      sysrad = rnd.rnd() * ai.sysradius()
      angle = rnd.rnd() * 2 * math.pi
      ai.pushtask("__goto_nobrake", vec2.new(math.cos(angle) * sysrad, math.sin(angle) * sysrad))
   end
end
