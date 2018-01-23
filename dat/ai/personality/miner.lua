-- Default task to run when idle
function idle ()
   field, ast = system.asteroid() -- Get a random asteroid in the system

   if ai.pilot():cargoFree() == 0 or field == nil then -- Leave this system
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

   else -- Mine the asteroid
      ai.pushtask( "mine", {field, ast} )
   end

end
