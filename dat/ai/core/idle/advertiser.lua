require 'ai.core.idle.civilian'

local idle_generic = idle

function idle ()
   local lastspammed = mem.adspamlast or 0
   local curtime = naev.ticks()
   if curtime - lastspammed > 15e3 then
      ai.pilot():broadcast(mem.ad)
      mem.adspamlast = curtime
   end
   return idle_generic ()
end

