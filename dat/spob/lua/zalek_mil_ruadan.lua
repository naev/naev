local luaspob = require "spob.lua.lib.spob"

luaspob.setup{
   std_land = 110, -- Impossible
   std_bribe = 100,
   msg_granted = {
      _([["Docking sequence transmitted."]]),
   },
   msg_notyet = {
      _([["Permission denied. Ruadan space is off-limits to you."]]),
   },
   msg_cantbribe = {
      _([["Money is irrelevant."]]),
   },
}
