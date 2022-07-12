local luaspob = require "spob.lua.lib.spob"

function init( spb )
   return luaspob.init( spb, {
      std_land = 50,
      std_bribe = 100,
      msg_granted = {
         _([["Docking sequence transmitted."]]),
      },
      msg_notyet = {
         _([["Authorization level too low to grant access."]]),
         _([["Authorization permissions insufficient for landing."]]),
         _([["Landing permission not found in database."]]),
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
