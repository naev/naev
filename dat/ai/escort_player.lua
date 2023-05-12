require 'ai.escort'

-- Settings
mem.aggressive = true

function create ()
end

function idle ()
   local pp = player.pilot()
   ai.pushtask( "follow", pp )
end
