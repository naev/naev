local luaspob = require "spob.lua.lib.spob"

luaspob.setup{
   std_land = 50,
   std_bribe = 100,
   msg_granted = {
      _("Permission to land granted."),
   },
   msg_notyet = {
      _([["Permission denied. You're not truly one of us."]]),
   },
   msg_cantbribe = {
      _([["We don't need your money, outsider."]]),
   },
}
