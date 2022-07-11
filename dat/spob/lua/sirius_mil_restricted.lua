local luaspob = require "spob.lua.lib.spob"

function load( spb )
   return luaspob.setup( spb, {
      std_land = 50,
      std_bribe = 100,
      msg_granted = {
         _("Landing access authorized."),
      },
      msg_notyet = {
         _([["Only the faithful may land here. Request denied."]]),
      },
      msg_cantbribe = {
         _([["The faithful will never be swayed by money."]]),
      },
   } )
end

can_land = luaspob.can_land
comm = luaspob.comm
