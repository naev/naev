local luaspob = require "spob.lua.lib.spob"

luaspob.setup{
   std_land = 50,
   std_bribe = 100,
   msg_granted = {
      _("Military clearance approved."),
      _("Military authorization cleared."),
   },
   msg_notyet = {
      _([["You are not authorized to land here, citizen."]]),
      _([["Landing is only for approved personnel."]]),
      _([["Military authorization required for landing access."]]),
   },
   msg_cantbribe = {
      _([["Don't attempt to bribe an Empire official, citizen."]]),
      _([["I'll let this one slide, but don't try this again, citizen."]]),
   },
}
