require 'ai.core.core'

-- Settings
mem.control_rate = 0.5 -- Lower control rate
mem.aggressive = true
mem.land_planet = false

function create ()
   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system
   mem.comm_no = _("No response.")
   create_post()
end

-- Default task to run when idle
function idle ()
   local field, ast = system.asteroid() -- Get a random asteroid in the system
   if field then
      ai.pushtask( "mine", {field, ast} )
   end
end

