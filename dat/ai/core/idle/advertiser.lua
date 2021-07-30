require 'ai.core.idle.civilian'

local idle_generic = idle

function idle ()
   ai.pilot():broadcast(mem.ad)
   return idle_generic ()
end

