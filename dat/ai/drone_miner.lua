require 'ai.core.core'

-- Settings
mem.control_rate = 0.5 -- Lower control rate
mem.aggressive = true
mem.land_planet = false

function create ()
   mem.comm_no = _("No response.")
   create_post()
end

-- Default task to run when idle
-- luacheck: globals idle (AI Task functions passed by name)
function idle ()
   local ast = asteroid.get() -- Get a random asteroid in the system
   if ast then
      ai.pushtask( "mine", ast )
   end
end
