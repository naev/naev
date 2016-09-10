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
       if not mem.boss then -- Pilot nover had a boss
           mem.boss = ai.getBoss()
       end
       -- If the boss exists, follow him
       if mem.boss and mem.boss:exists() then
           mem.angle = rnd.rnd( 360 )
           mem.radius = rnd.rnd( 100, 200 )
           ai.pushtask("follow_accurate",mem.boss)
       else  -- The pilot has no boss, he chooses his way
           sysrad = rnd.rnd() * system.cur():radius()
           angle = rnd.rnd() * 2 * math.pi
           ai.pushtask("__goto_nobrake", vec2.new(math.cos(angle) * sysrad, math.sin(angle) * sysrad))
       end
   end
   mem.loiter = mem.loiter - 1
end

-- Settings
mem.land_friendly = true -- Land on only friendly by default
