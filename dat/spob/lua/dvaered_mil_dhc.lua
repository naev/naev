local luaspob = require "spob.lua.lib.spob"

luaspob.setup{
   std_land = 90,
   std_bribe = 100,
   msg_granted = {
      _([["Permission to land granted, captain."]]),
   },
   msg_notyet = {
      _([["Only high ranking personnel allowed. Landing request denied."]]),
   },
   msg_cantbribe = {
      _([["Money won't buy you access to our restricted facilities, citizen."]]),
   },
}
