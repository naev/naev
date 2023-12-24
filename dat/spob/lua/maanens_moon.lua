local luaspob = require "spob.lua.lib.spob"

luaspob.setup{
   std_land = 50,
   std_bribe = 0,
   msg_granted = {
      _("Permission to land granted."),
   },
   msg_notyet = {
      _("Access to Maanen's Moon denied."),
      _("You need special clearance to land on Maanen's Moon."),
   },
   msg_cantbribe = {
      _([["Don't attempt to bribe an Empire official, pilot."]]),
   },
}
