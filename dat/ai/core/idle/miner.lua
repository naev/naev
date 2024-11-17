-- Default task to run when idle
function idle ()
   local ast = asteroid.get( mem.mining_field ) -- Get a random asteroid in the system (or current mining field)

   if mem.force_leave or (ai.pilot():cargoFree()==0) or (ast==nil) then -- Leave this system
      local planet = ai.landspob( mem.land_friendly )
      -- planet must exist
      if planet == nil then
         ai.settimer(0, rnd.uniform(1.0, 3.0))
         ai.pushtask("enterdelay")
      else
         ai.pushtask("hyperspace")
         ai.pushtask("land", planet)
      end

   else -- Mine the asteroid
      mem.mining_field = ast:field()
      ai.pushtask( "mine", ast )
   end
end
