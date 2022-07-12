local luaspob = require "spob.lua.lib.spob"

function init( spb )
   return luaspob.init( spb, {
      std_land = 110, -- Impossible
      std_bribe = 100,
      msg_granted = {
         _([["Docking sequence transmitted."]]),
      },
      msg_notyet = {
         _([["Permission denied. Ruadan space is off-limits to you."]]),
      },
      msg_cantbribe = {
         _([["Money is irrelevant."]]),
      },
   } )
end

load = luaspob.load
unload = luaspob.unload
can_land = luaspob.can_land
comm = luaspob.comm
