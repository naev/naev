-- Default task to run when idle
function idle ()
   r = rnd.rnd()
   if r < 0.34 then -- Try to leave.
       local planet = ai.landplanet()
       -- planet must exist
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
   else -- Stay. Have a beer.
      -- sysrad = rnd.rnd() * system.cur():radius() -- TODO: Make this work somehow
      sysrad = 3500 -- This is the radius of most systems anyway.
      angle = rnd.rnd() * 2 * math.pi
      ai.pushtask("__goto_nobrake", vec2.new(math.cos(angle) * sysrad, math.sin(angle) * sysrad))
   end
end