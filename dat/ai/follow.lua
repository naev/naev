require 'ai.escort'

-- Settings
mem.aggressive = false
mem.land_planet = false
mem.norun = true

function create ()
   create_pre()
   mem.bribe_no = true
   mem.refuel_no = true
   create_post()
   mem.atk = require "ai.core.attack.pacifist"
end

function idle ()
   local l = ai.pilot():leader()
   if l then
      ai.pushtask( "follow", l )
   else
      ai.settimer( 0, 5 )
      ai.pushtask( "idle_wait", l )
   end
end
