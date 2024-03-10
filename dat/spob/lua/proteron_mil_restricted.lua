local luaspob = require "spob.lua.lib.spob"

luaspob.setup{
   std_land = 50,
   std_bribe = 100,
   msg_granted = {
      _("Landing access authorized."),
   },
   msg_notyet = {
      _([["You are not authorized to land here."]]),
   },
   msg_cantbribe = {
      _([["The Sovereign Proteron Autarchy does not take bribes."]]),
   },
}
