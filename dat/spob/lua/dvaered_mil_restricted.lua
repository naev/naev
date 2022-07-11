local luaspob = require "spob.lua.lib.spob"

function load( spb )
   return luaspob.setup( spb, {
      std_land = 50,
      std_bribe = 100,
      msg_granted = {
         _("Permission to land granted."),
      },
      msg_notyet = {
         _([["Your rank is too low, citizen. Access denied."]]),
      },
      msg_cantbribe = {
         _([["Money won't buy you access to our restricted facilities, citizen."]]),
      },
   } )
end

can_land = luaspob.can_land
comm = luaspob.comm
