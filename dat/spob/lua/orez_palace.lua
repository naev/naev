local luaspob = require "spob.lua.lib.spob"

luaspob.setup{
   std_land = 50,
   std_bribe = 100,
   msg_granted = {
      _([["Permission to land granted."]]),
   },
   msg_notyet = {
      _([["You are not yet welcome at O'rez Palace."]]),
   },
   msg_cantbribe = {
      _([["The little money you can offer is of no interest to us."]]),
   },
}
