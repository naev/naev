local luaspob = require "spob.lua.lib.spob"

luaspob.setup{
   std_land = 50,
   std_bribe = 100,
   msg_granted = {
      _([["Docking sequence transmitted."]]),
   },
   msg_notyet = {
      _([["Authorization level too low to grant access."]]),
      _([["Authorization permissions insufficient for landing."]]),
      _([["Landing permission not found in database."]]),
   },
   msg_cantbribe = {
      _([["Money is irrelevant."]]),
   },
}
