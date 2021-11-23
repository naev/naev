require 'ai.escort'

-- Settings
mem.aggressive = true

function create ()
end

-- luacheck: globals idle (AI Task functions passed by name)
function idle ()
   local pp = player.pilot()
   ai.pushtask( "follow", pp )
end
