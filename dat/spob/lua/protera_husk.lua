local luaspob = require "spob.lua.lib.spob"

luaspob.setup{
   std_land = -101,
   std_bribe = -101,
   msg_granted = {
      _("Permission to land granted."),
   },
   msg_notyet = {
      _([["Your rank is too low, citizen. Access denied."]]),
   },
   msg_cantbribe = {
      _([["Money won't buy you access to our restricted facilities, citizen."]]),
   },
}
