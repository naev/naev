-- Default task to run when idle
-- luacheck: globals idle (AI Task functions passed by name)
function idle ()
   local field, ast = system.asteroid() -- Get a random asteroid in the system

   if ai.pilot():cargoFree() == 0 or field == nil then -- Leave this system
      local planet = ai.landplanet( mem.land_friendly )
      -- planet must exist
      if planet == nil then
         ai.settimer(0, rnd.uniform(1.0, 3.0))
         ai.pushtask("enterdelay")
      else
         ai.pushtask("hyperspace")
         ai.pushtask("land", planet)
      end

   else -- Mine the asteroid
      ai.pushtask( "mine", {field, ast} )
   end

end
