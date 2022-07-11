local luaspob = require "spob.lua.lib.spob"

function load( spb )
   return luaspob.setup( spb, {
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

can_land = luaspob.can_land
comm = luaspob.comm
