local luaspob = require "spob.lua.lib.spob"

luaspob.setup{
   std_land = 90,
   std_bribe = 100,
   msg_granted = {
      _("Permission to land granted."),
   },
   msg_notyet = {
      _([["Only friends of the Soromid may set foot on Kataka."]]),
   },
   msg_cantbribe = {
      _([["We don't need your money, outsider."]]),
   },
}
