require 'ai.escort'

-- Settings
mem.aggressive = true

function create ()
   mem.bribe_no = true
   mem.refuel_no = true
end

function idle ()
   local pp = player.pilot()
   ai.pushtask( "follow", pp )
end
