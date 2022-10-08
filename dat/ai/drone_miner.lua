require 'ai.core.core'

-- Settings
mem.control_rate = 0.5 -- Lower control rate
mem.aggressive = true
mem.land_planet = false

function create ()
   create_pre()
   mem.comm_no = _("No response.")
   create_post()
end

-- Default task to run when idle
function idle ()
   local ast = asteroid.get( mem.mining_field ) -- Get a random asteroid in the system (or current mining field)
   if ast then
      mem.mining_field = ast:field()
      ai.pushtask( "mine", ast )
   end
end
