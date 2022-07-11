local luaspob = require "spob.lua.lib.spob"

function load( spb )
   return luaspob.setup( spb, {
      std_land = 50,
      std_bribe = 100,
      msg_granted = {
         _("Landing access authorized."),
      },
      msg_notyet = {
         _([["You are not authorized to land here."]]),
      },
      msg_cantbribe = {
         _([["Don't attempt to bribe an Empire official, pilot."]]),
      },
   } )
end

can_land = luaspob.can_land
comm = luaspob.comm
